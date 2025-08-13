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
    bool ImageManager::has(std::string_view logical) const
    {
        std::scoped_lock lock(mtx_);
        auto [key, _] = resolveLogical_(logical);
        if (key.empty()) return false;
        return images_.find(key) != images_.end();
    }

    bool ImageManager::isLoaded(std::string_view logical) const
    {
        std::scoped_lock lock(mtx_);
        auto [key, _] = resolveLogical_(logical);
        auto it = images_.find(key);
        return it != images_.end() && it->second.obj && it->second.obj->valid();
    }

    bool ImageManager::existsOnDisk(std::string_view logical) const
    {
        std::scoped_lock lock(mtx_);
        auto [key, physical] = resolveLogical_(logical);
        auto it = images_.find(key);
        if (it != images_.end() && it->second.memory_backed) return true; // memory-backed: treat as existing
        return fs::exists(physical);
    }

    std::optional<fs::path> ImageManager::resolve(std::string_view logical) const
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
    ImageManager::Ptr ImageManager::get(std::string_view logical)
    {
        std::scoped_lock lock(mtx_);
        auto [key, _] = resolveLogical_(logical);
        if (key.empty()) return {};
        auto it = images_.find(key);
        if (it == images_.end()) return {};
        it->second.last_access = Clock::now();
        return it->second.obj;
    }

    ImageManager::Ptr ImageManager::load(std::string_view logical, const ImageLoadOptions& opt)
    {
        std::scoped_lock lock(mtx_);
        auto [key, physical] = resolveLogical_(logical);
        if (key.empty()) return {};

        auto& rec = images_[key];
        if (!rec.obj)
        {
            rec.obj = std::make_shared<Image>();
            rec.path = physical;
            rec.label = isAliasSyntax_(logical) ? std::string{ logical }
                                                : aliasLabelFor_(physical);
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

    ImageManager::Ptr ImageManager::load(std::string_view logical) { return load(logical, defaults_); }

    // ----- PATH versions (distinct names to avoid overload ambiguity) --------
    ImageManager::Ptr ImageManager::getPath(const fs::path& path) { return get(path.string()); }

    ImageManager::Ptr ImageManager::loadPath(const fs::path& path, const ImageLoadOptions& opt)
    {
        return load(path.string(), opt);
    }
    ImageManager::Ptr ImageManager::loadPath(const fs::path& path)
    {
        return load(path.string(), defaults_);
    }

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
    std::vector<ImageManager::Ptr> ImageManager::loadAllPaths(const std::vector<fs::path>& paths, const ImageLoadOptions& opt)
    {
        std::vector<Ptr> out; out.reserve(paths.size());
        for (auto const& p : paths) out.push_back(loadPath(p, opt));
        return out;
    }
    std::vector<ImageManager::Ptr> ImageManager::loadAllPaths(const std::vector<fs::path>& paths) { return loadAllPaths(paths, defaults_); }

    // Directory preload
    int ImageManager::preloadDir(const fs::path& dir, const std::vector<std::string>& exts, bool recursive)
    {
        int count = 0;
        if (!fs::exists(dir)) return 0;

        // Pre-normalize extension list once (lowercase)
        std::vector<std::string> exts_lower; exts_lower.reserve(exts.size());
        for (const auto& sx : exts)
        {
            std::string s = sx;
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
            exts_lower.push_back(std::move(s));
        }

        auto match = [&](const fs::path& p)
            {
                std::string e = p.extension().string();
                std::transform(e.begin(), e.end(), e.begin(), [](unsigned char c) { return (char)std::tolower(c); });
                for (const auto& s : exts_lower) if (e == s) return true;
                return false;
            };

        if (recursive)
        {
            for (auto& entry : fs::recursive_directory_iterator(dir))
            {
                if (entry.is_regular_file() && match(entry.path())) { loadPath(entry.path()); ++count; }
            }
        }
        else
        {
            for (auto& entry : fs::directory_iterator(dir))
            {
                if (entry.is_regular_file() && match(entry.path())) { loadPath(entry.path()); ++count; }
            }
        }
        return count;
    }

    // ---------------------------- reload / housekeeping -----------------------
    bool ImageManager::reload(std::string_view logical)
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

    bool ImageManager::reload(std::string_view logical, const ImageLoadOptions& opt)
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

    bool ImageManager::reloadPath(const fs::path& path) { return reload(path.string()); }

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

    bool ImageManager::unload(std::string_view logical)
    {
        std::scoped_lock lock(mtx_);
        auto [key, _] = resolveLogical_(logical);
        return !key.empty() && images_.erase(key) > 0;
    }

    bool ImageManager::unloadPath(const fs::path& path) { return unload(path.string()); }

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

    bool ImageManager::isAliasSyntax_(std::string_view s) { return s.find("://") != std::string::npos; }

    std::string ImageManager::aliasLabelFor_(const fs::path& physical) const
    {
        // Canonicalize for stable comparisons
        std::error_code ec;
        fs::path phys_can = fs::weakly_canonical(physical, ec);
        if (ec) phys_can = physical.lexically_normal();
        const std::string phys = phys_can.generic_string();

        std::string best_label;
        std::size_t best_len = 0;

        for (const auto& [alias, roots] : aliases_)
        {
            for (const auto& root : roots)
            {
                fs::path root_can = fs::weakly_canonical(root, ec);
                if (ec) root_can = root.lexically_normal();
                const std::string base = root_can.generic_string();

                // prefix match with boundary (exact dir or followed by '/')
                if (phys.size() >= base.size() &&
                    phys.compare(0, base.size(), base) == 0 &&
                    (phys.size() == base.size() || phys[base.size()] == '/'))
                {

                    // relative part (skip the '/')
                    const std::string rel =
                        (phys.size() == base.size()) ? std::string{} : phys.substr(base.size() + 1);

                    const std::string candidate = alias + "://" + rel;
                    if (base.size() > best_len)
                    {
                        best_len = base.size();
                        best_label = candidate;
                    }
                }
            }
        }

        // Fallback: no alias root matched; show the physical path
        return best_label.empty() ? phys : best_label;
    }

    std::pair<std::string, fs::path> ImageManager::resolveLogical_(std::string_view logical) const
    {
        if (!isAliasSyntax_(logical))
        {
            const fs::path p = std::string{ logical };
            return { normalize_(p), p };
        }
        const auto pos = logical.find("://");
        const std::string alias = std::string{ logical.substr(0, pos) };
        fs::path relative = std::string{ logical.substr(pos + 3) };

        auto it = aliases_.find(alias);
        if (it == aliases_.end() || it->second.empty())
        {
            const fs::path p = std::string{ logical };
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
#include "../Assets/Fonts/Icons/IconsFontAwesome5Pro.h"

    namespace
    {
        inline ImVec2 DrawTexturePreview(ImTextureID tex, int iw, int ih,
            float max_w, float max_h, bool border)
        {
            if (!tex || iw <= 0 || ih <= 0) return ImVec2(0, 0);
            if (max_w <= 0) max_w = ImGui::GetContentRegionAvail().x;
            if (max_h <= 0) max_h = static_cast<float>(ih);
            const float sx = max_w / static_cast<float>(iw);
            const float sy = max_h / static_cast<float>(ih);
            const float s = (max_h > 0) ? (sx < sy ? sx : sy) : sx;
            const float w = static_cast<float>(iw) * s;
            const float h = static_cast<float>(ih) * s;
            const ImVec4 border_col = border ? ImGui::GetStyleColorVec4(ImGuiCol_Border)
                : ImVec4(0, 0, 0, 0);
            ImGui::Image(tex, ImVec2(w, h), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), border_col);
            return ImVec2(w, h);
        }
    }

    bool DrawImageManagerPanel(ImageManager& mgr, ImagePanelState& state)
    {
        if (ImGui::Button(ICON_FA_REDO " Reload Changed")) { (void)mgr.reloadChanged(); }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_BROOM " Prune Unused")) { (void)mgr.pruneUnused(); }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_TRASH " Clear All")) { mgr.clear(); }
        ImGui::SameLine();
        ImGui::Checkbox("Inline Preview", &state.preview_inline);
        ImGui::SameLine();
        ImGui::Checkbox("Preview Window", &state.preview_window);

        ImGui::Separator();

        bool selection_changed = false;
        auto rows = mgr.snapshot();

        const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders
            | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;

        if (ImGui::BeginTable("##image_mgr", 7, flags, ImVec2(0, state.table_height)))
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

                ImGui::TableSetColumnIndex(0);
                const bool is_selected = (state.selected_key == r.key);
                if (ImGui::Selectable((r.label + "##row").c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns))
                {
                    state.selected_key = r.key;
                    state.selected_label = r.label;
                    selection_changed = true;
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted(r.path.c_str());
                    ImGui::Text("Size: %dx%d", r.w, r.h);
                    ImGui::EndTooltip();
                }

                ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(r.path.c_str());
                ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(r.exists ? "Yes" : "No");
                ImGui::TableSetColumnIndex(3); ImGui::Text("%dx%d", r.w, r.h);
                ImGui::TableSetColumnIndex(4); ImGui::Text("%d", r.refs);
                ImGui::TableSetColumnIndex(5); ImGui::TextUnformatted(r.memory_backed ? "Y" : "");
                ImGui::TableSetColumnIndex(6);
                if (!r.memory_backed)
                {
                    if (ImGui::SmallButton((std::string(ICON_FA_REDO " Reload##") + r.key).c_str()))
                    {
                        mgr.reload(r.key);
                    }
                }
                else
                {
                    ImGui::BeginDisabled();
                    ImGui::SmallButton((std::string(ICON_FA_REDO " Reload##") + r.key).c_str());
                    ImGui::EndDisabled();
                }
            }
            ImGui::EndTable();
        }

        // Inline preview below the table
        if (state.preview_inline && !state.selected_key.empty())
        {
            ImGui::Separator();
            ImGui::TextUnformatted("Preview:");
            ImGui::SliderFloat("Max Height", &state.preview_max_height, 64.0f, 1024.0f, "%.0f px");
            auto img = mgr.get(state.selected_key);
            if (img && img->valid())
            {
                DrawTexturePreview(img->ImGuiID(), img->width(), img->height(),
                    ImGui::GetContentRegionAvail().x, state.preview_max_height, true);
            }
            else
            {
                ImGui::TextDisabled("No image available.");
            }
        }

        // Separate preview window
        if (state.preview_window && !state.selected_key.empty())
        {
            ImGui::SetNextWindowSize(ImVec2(480, 400), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Image Preview", &state.preview_window))
            {
                ImGui::TextUnformatted(state.selected_label.c_str());
                ImGui::Separator();
                ImGui::SliderFloat("Max Height##wnd", &state.preview_max_height, 64.0f, 2048.0f, "%.0f px");
                auto img = mgr.get(state.selected_key);
                if (img && img->valid())
                {
                    DrawTexturePreview(img->ImGuiID(), img->width(), img->height(),
                        ImGui::GetContentRegionAvail().x, state.preview_max_height, true);
                }
                else
                {
                    ImGui::TextDisabled("No image available.");
                }
            }
            ImGui::End();
        }

        return selection_changed;
    }

    void DrawImageManagerPanel(ImageManager& mgr)
    {
        static ImagePanelState s; // persistent selection & prefs
        (void)DrawImageManagerPanel(mgr, s);
    }

#endif // IMGUI_VERSION

} // namespace assets
