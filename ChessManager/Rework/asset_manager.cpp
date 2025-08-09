// ===== File: AssetManager.cpp =====
#include "asset_manager.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>

namespace assets
{

    // ---------------------------- ctor / defaults ----------------------------
    ImageManager::ImageManager() = default;

    void ImageManager::setDefaultOptions(const ImageLoadOptions& o) { std::scoped_lock lock(mtx_); defaults_ = o; }
    const ImageLoadOptions& ImageManager::defaultOptions() const { return defaults_; }

    // ---------------------------- aliases ------------------------------------
    void ImageManager::addAlias(const std::string& name, const fs::path& root)
    {
        std::scoped_lock lock(mtx_);
        aliases_[name].push_back(canonDir_(root));
    }
    void ImageManager::setAlias(const std::string& name, std::vector<fs::path> roots)
    {
        std::scoped_lock lock(mtx_);
        auto& v = aliases_[name]; v.clear(); v.reserve(roots.size());
        for (auto& r : roots) v.push_back(canonDir_(r));
    }
    bool ImageManager::removeAlias(const std::string& name)
    {
        std::scoped_lock lock(mtx_);
        return aliases_.erase(name) > 0;
    }
    void ImageManager::clearAliases()
    {
        std::scoped_lock lock(mtx_);
        aliases_.clear();
    }
    std::vector<std::string> ImageManager::listAliases() const
    {
        std::scoped_lock lock(mtx_);
        std::vector<std::string> out; out.reserve(aliases_.size());
        for (auto const& kv : aliases_) out.push_back(kv.first);
        std::sort(out.begin(), out.end());
        return out;
    }

    // ---------------------------- helpers ------------------------------------
    bool ImageManager::has(const std::string& logical) const
    {
        std::scoped_lock lock(mtx_);
        auto [key, _] = resolveLogical_(logical);
        if (key.empty()) return false;
        return images_.find(key) != images_.end();
    }

    bool ImageManager::isLoaded(const std::string& logical) const
    {
        std::scoped_lock lock(mtx_);
        auto [key, _] = resolveLogical_(logical);
        auto it = images_.find(key);
        return it != images_.end() && it->second.obj && it->second.obj->valid();
    }

    bool ImageManager::existsOnDisk(const std::string& logical) const
    {
        std::scoped_lock lock(mtx_);
        auto [key, physical] = resolveLogical_(logical);
        auto it = images_.find(key);
        if (it != images_.end() && it->second.memory_backed) return true; // memory-backed: treat as existing
        return fs::exists(physical);
    }

    std::optional<fs::path> ImageManager::resolve(const std::string& logical) const
    {
        auto [_, physical] = resolveLogical_(logical);
        return physical;
    }

    std::vector<std::string> ImageManager::findMissing(const std::vector<std::string>& logicals) const
    {
        std::vector<std::string> missing;
        missing.reserve(logicals.size());
        for (auto const& s : logicals) if (!existsOnDisk(s)) missing.push_back(s);
        return missing;
    }

    std::size_t ImageManager::totalApproxBytes() const
    {
        std::scoped_lock lock(mtx_);
        std::size_t sum = 0;
        for (auto const& kv : images_)
        {
            auto& rec = kv.second;
            if (rec.obj && rec.obj->valid())
            {
                int ch = rec.obj->channels() > 0 ? rec.obj->channels() : 4;
                sum += static_cast<std::size_t>(rec.obj->width()) * static_cast<std::size_t>(rec.obj->height()) * static_cast<std::size_t>(ch);
            }
        }
        return sum;
    }

    // ---------------------------- get / load ---------------------------------
    ImageManager::Ptr ImageManager::get(const std::string& logical)
    {
        std::scoped_lock lock(mtx_);
        auto [key, _] = resolveLogical_(logical);
        if (key.empty()) return {};
        auto it = images_.find(key);
        if (it == images_.end()) return {};
        it->second.last_access = Clock::now();
        return it->second.obj;
    }
    ImageManager::Ptr ImageManager::get(const fs::path& path) { return get(path.string()); }

    ImageManager::Ptr ImageManager::load(const std::string& logical, const ImageLoadOptions& opt)
    {
        std::scoped_lock lock(mtx_);
        auto [key, physical] = resolveLogical_(logical);
        if (key.empty()) return {};

        auto& rec = images_[key];
        if (!rec.obj)
        {
            rec.obj = std::make_shared<Image>();
            rec.path = physical;
            rec.label = logical;
            rec.options = opt;
        }

        if (rec.obj->width() == 0 && rec.obj->height() == 0)
        {
            loadInto_(*rec.obj, rec.path, rec.options);
            rec.timestamp = safeTimestamp_(rec.path);
        }
        rec.last_access = Clock::now();
        return rec.obj;
    }

    ImageManager::Ptr ImageManager::load(const fs::path& path, const ImageLoadOptions& opt) { return load(path.string(), opt); }
    ImageManager::Ptr ImageManager::load(const std::string& logical) { return load(logical, defaults_); }
    ImageManager::Ptr ImageManager::load(const fs::path& path) { return load(path.string(), defaults_); }

    ImageManager::Ptr ImageManager::loadFromMemory(const std::string& key, const unsigned char* bytes, int byte_count,
        const ImageLoadOptions& opt)
    {
        std::scoped_lock lock(mtx_);
        auto& rec = images_[key];
        if (!rec.obj) rec.obj = std::make_shared<Image>();
        rec.path = key; // pseudo-path label for UI
        rec.label = key;
        rec.memory_backed = true;
        rec.options = opt;

        loadIntoMemory_(*rec.obj, bytes, byte_count, rec.options);
        rec.timestamp = {};
        rec.last_access = Clock::now();
        return rec.obj;
    }
    ImageManager::Ptr ImageManager::loadFromMemory(const std::string& key, const unsigned char* bytes, int byte_count)
    {
        return loadFromMemory(key, bytes, byte_count, defaults_);
    }

    // Batch
    std::vector<ImageManager::Ptr> ImageManager::loadAll(const std::vector<std::string>& logicals, const ImageLoadOptions& opt)
    {
        std::vector<Ptr> out; out.reserve(logicals.size());
        for (auto const& s : logicals) out.push_back(load(s, opt));
        return out;
    }
    std::vector<ImageManager::Ptr> ImageManager::loadAll(const std::vector<std::string>& logicals) { return loadAll(logicals, defaults_); }
    std::vector<ImageManager::Ptr> ImageManager::loadAll(const std::vector<fs::path>& paths, const ImageLoadOptions& opt)
    {
        std::vector<Ptr> out; out.reserve(paths.size());
        for (auto const& p : paths) out.push_back(load(p.string(), opt));
        return out;
    }
    std::vector<ImageManager::Ptr> ImageManager::loadAll(const std::vector<fs::path>& paths) { return loadAll(paths, defaults_); }

    // Directory preload
    int ImageManager::preloadDir(const fs::path& dir, const std::vector<std::string>& exts, bool recursive)
    {
        int count = 0;
        if (!fs::exists(dir)) return 0;
        // Pre-normalize extensions
        std::vector<std::string> exts_lower;
        exts_lower.reserve(exts.size());
        for (const auto& sx : exts)
        {
            std::string s = sx; // non-const copy
            std::transform(s.begin(), s.end(), s.begin(),
                [](unsigned char c) { return (char)std::tolower(c); });
            exts_lower.push_back(std::move(s));
        }

        auto match = [&](const fs::path& p)
            {
                std::string e = p.extension().string();
                std::transform(e.begin(), e.end(), e.begin(),
                    [](unsigned char c) { return (char)std::tolower(c); });
                for (const auto& s : exts_lower)
                {
                    if (e == s) return true;
                }
                return false;
            };
        if (recursive)
        {
            for (auto& entry : fs::recursive_directory_iterator(dir))
            {
                if (entry.is_regular_file() && match(entry.path())) { load(entry.path()); ++count; }
            }
        }
        else
        {
            for (auto& entry : fs::directory_iterator(dir))
            {
                if (entry.is_regular_file() && match(entry.path())) { load(entry.path()); ++count; }
            }
        }
        return count;
    }

    // ---------------------------- reload / housekeeping -----------------------
    bool ImageManager::reload(const fs::path& path) { return reload(path.string()); }

    bool ImageManager::reload(const std::string& logical)
    {
        std::scoped_lock lock(mtx_);
        auto [key, physical] = resolveLogical_(logical);
        if (key.empty()) return false;
        auto it = images_.find(key);
        if (it == images_.end()) return false;
        Record& rec = it->second;
        if (rec.memory_backed) return false; // memory-backed entries can't be file-reloaded
        if (!rec.obj) rec.obj = std::make_shared<Image>();

        Image tmp(rec.path, rec.options.flip_vertically, rec.options.force_channels, rec.options.generate_mipmaps);
        *rec.obj = std::move(tmp);
        rec.timestamp = safeTimestamp_(rec.path);
        rec.last_access = Clock::now();
        return rec.obj->valid();
    }

    bool ImageManager::reload(const std::string& logical, const ImageLoadOptions& opt)
    {
        std::scoped_lock lock(mtx_);
        auto [key, physical] = resolveLogical_(logical);
        if (key.empty()) return false;
        auto it = images_.find(key);
        if (it == images_.end()) return false;
        Record& rec = it->second;
        if (rec.memory_backed) return false;
        rec.options = opt; // update stored options

        Image tmp(rec.path, rec.options.flip_vertically, rec.options.force_channels, rec.options.generate_mipmaps);
        *rec.obj = std::move(tmp);
        rec.timestamp = safeTimestamp_(rec.path);
        rec.last_access = Clock::now();
        return rec.obj->valid();
    }

    int ImageManager::reloadChanged()
    {
        std::scoped_lock lock(mtx_);
        int reloaded = 0;
        for (auto& [key, rec] : images_)
        {
            if (rec.memory_backed) continue;
            const auto ts = safeTimestamp_(rec.path);
            if (ts != fs::file_time_type{} && ts != rec.timestamp)
            {
                Image tmp(rec.path, rec.options.flip_vertically, rec.options.force_channels, rec.options.generate_mipmaps);
                *rec.obj = std::move(tmp);
                rec.timestamp = ts;
                rec.last_access = Clock::now();
                ++reloaded;
            }
        }
        return reloaded;
    }

    int ImageManager::pruneUnused()
    {
        std::scoped_lock lock(mtx_);
        int removed = 0;
        for (auto it = images_.begin(); it != images_.end(); )
        {
            if (it->second.obj && it->second.obj.use_count() == 1)
            {
                it = images_.erase(it);
                ++removed;
            }
            else
            {
                ++it;
            }
        }
        return removed;
    }

    bool ImageManager::unload(const std::string& logical)
    {
        std::scoped_lock lock(mtx_);
        auto [key, _] = resolveLogical_(logical);
        return !key.empty() && images_.erase(key) > 0;
    }

    bool ImageManager::unload(const fs::path& path) { return unload(path.string()); }

    void ImageManager::clear()
    {
        std::scoped_lock lock(mtx_);
        images_.clear();
    }

    std::size_t ImageManager::size() const
    {
        std::scoped_lock lock(mtx_);
        return images_.size();
    }

    std::vector<ImageManager::DebugInfo> ImageManager::snapshot() const
    {
        std::scoped_lock lock(mtx_);
        std::vector<DebugInfo> out; out.reserve(images_.size());
        for (auto const& [key, rec] : images_)
        {
            DebugInfo d; d.key = key; d.label = rec.label; d.path = rec.path.string();
            d.exists = rec.memory_backed ? true : fs::exists(rec.path);
            d.refs = rec.obj ? (int)rec.obj.use_count() : 0;
            d.w = rec.obj ? rec.obj->width() : 0;
            d.h = rec.obj ? rec.obj->height() : 0;
            d.memory_backed = rec.memory_backed;
            out.push_back(std::move(d));
        }
        std::sort(out.begin(), out.end(), [](auto& a, auto& b) { return a.key < b.key; });
        return out;
    }

    // ---------------------------- private helpers -----------------------------
    std::string ImageManager::normalize_(const fs::path& p)
    {
        try { return fs::weakly_canonical(p).string(); }
        catch (...) { return p.lexically_normal().string(); }
    }

    fs::path ImageManager::canonDir_(const fs::path& p)
    {
        try { return fs::weakly_canonical(p); }
        catch (...) { return p.lexically_normal(); }
    }

    fs::file_time_type ImageManager::safeTimestamp_(const fs::path& p)
    {
        try { return fs::exists(p) ? fs::last_write_time(p) : fs::file_time_type{}; }
        catch (...) { return fs::file_time_type{}; }
    }

    void ImageManager::loadInto_(Image& dst, const fs::path& p, const ImageLoadOptions& opt)
    {
        Image tmp(p, opt.flip_vertically, opt.force_channels, opt.generate_mipmaps);
        dst = std::move(tmp);
    }

    void ImageManager::loadIntoMemory_(Image& dst, const unsigned char* bytes, int byte_count, const ImageLoadOptions& opt)
    {
        Image tmp; // default
        if (!tmp.loadFromMemory(bytes, byte_count, opt.flip_vertically, opt.force_channels, opt.generate_mipmaps))
        {
            std::cerr << "[ImageManager] Failed to load from memory bytes";
        }
        dst = std::move(tmp);
    }

    bool ImageManager::isAliasSyntax_(const std::string& s) { return s.find("://") != std::string::npos; }

    std::pair<std::string, fs::path> ImageManager::resolveLogical_(const std::string& logical) const
    {
        if (!isAliasSyntax_(logical))
        {
            const fs::path p = logical;
            return { normalize_(p), p };
        }
        const auto pos = logical.find("://");
        const std::string alias = logical.substr(0, pos);
        fs::path relative = logical.substr(pos + 3);

        auto it = aliases_.find(alias);
        if (it == aliases_.end() || it->second.empty())
        {
            const fs::path p = logical;
            return { normalize_(p), p };
        }

        for (auto const& root : it->second)
        {
            fs::path candidate = root / relative;
            if (fs::exists(candidate))
            {
                return { normalize_(candidate), candidate };
            }
        }
        fs::path fallback = it->second.front() / relative;
        return { normalize_(fallback), fallback };
    }

    // ---------------------------- ImGui panel ---------------------------------
#ifdef IMGUI_VERSION
#include "imgui.h"

    void DrawImageManagerPanel(ImageManager& mgr)
    {
        if (ImGui::Button("Reload Changed")) { (void)mgr.reloadChanged(); }
        ImGui::SameLine();
        if (ImGui::Button("Prune Unused")) { (void)mgr.pruneUnused(); }
        ImGui::SameLine();
        if (ImGui::Button("Clear All")) { mgr.clear(); }

        ImGui::Separator();

        auto rows = mgr.snapshot();
        if (ImGui::BeginTable("##image_mgr", 7, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.35f);
            ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch, 0.45f);
            ImGui::TableSetupColumn("Exists");
            ImGui::TableSetupColumn("Size");
            ImGui::TableSetupColumn("Refs");
            ImGui::TableSetupColumn("Mem");
            ImGui::TableSetupColumn("Actions");
            ImGui::TableHeadersRow();

            for (auto const& r : rows)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(r.label.c_str());
                ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(r.path.c_str());
                ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(r.exists ? "Yes" : "No");
                ImGui::TableSetColumnIndex(3); ImGui::Text("%dx%d", r.w, r.h);
                ImGui::TableSetColumnIndex(4); ImGui::Text("%d", r.refs);
                ImGui::TableSetColumnIndex(5); ImGui::TextUnformatted(r.memory_backed ? "Y" : "");
                ImGui::TableSetColumnIndex(6);
                if (!r.memory_backed)
                {
                    if (ImGui::SmallButton((std::string("Reload##") + r.key).c_str()))
                    {
                        mgr.reload(r.key);
                    }
                }
                else
                {
                    ImGui::BeginDisabled();
                    ImGui::SmallButton((std::string("Reload##") + r.key).c_str());
                    ImGui::EndDisabled();
                }
            }
            ImGui::EndTable();
        }
    }
#endif // IMGUI_VERSION

} // namespace assets
