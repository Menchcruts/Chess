// ===== File: AssetManager.hpp =====
#pragma once
// Manages loaded image assets (Images only). Source in AssetManager.cpp.
// - Single source of truth per resolved physical path (cache)
// - In-place reload (shared_ptr<Image> stays valid)
// - Hot-reload via last_write_time()
// - Batch loading and path aliasing (e.g., textures://ui/button.png)
// - Optional ImGui debug panel (decl only here) 
//
// Requirements:
// - C++17+
// - Your Image class from "ImageRework.h" (RAII + move-assign)
// - OpenGL context must be current when (re)loading

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>

#include "ImageRework.h"

namespace assets
{

    namespace fs = std::filesystem;

    struct ImageLoadOptions
    {
        bool flip_vertically = false;
        int  force_channels = 4;   // 0 = keep source; 3 = RGB; 4 = RGBA
        bool generate_mipmaps = true;
    };

    class ImageManager
    {
    public:
        using Ptr = std::shared_ptr<Image>;

        ImageManager();

        // Defaults ---------------------------------------------------------
        void setDefaultOptions(const ImageLoadOptions& o);
        const ImageLoadOptions& defaultOptions() const;

        // Aliases ----------------------------------------------------------
        // addAlias("textures", "assets/textures") then load("textures://ui/button.png");
        void addAlias(const std::string& name, const fs::path& root);
        void setAlias(const std::string& name, std::vector<fs::path> roots);
        bool removeAlias(const std::string& name);
        void clearAliases();
        std::vector<std::string> listAliases() const;

        // Helpers ----------------------------------------------------------
        bool has(const std::string& logical) const;          // present in cache
        bool isLoaded(const std::string& logical) const;     // present and valid Image
        bool existsOnDisk(const std::string& logical) const; // resolves alias and checks filesystem or memory-backed
        std::optional<fs::path> resolve(const std::string& logical) const; // resolve logical to physical path
        std::vector<std::string> findMissing(const std::vector<std::string>& logicals) const; // which don't exist on disk
        std::size_t totalApproxBytes() const; // rough VRAM usage estimate (w*h*4)

        // Get / Load -------------------------------------------------------
        Ptr get(const std::string& logical);
        Ptr get(const fs::path& path);

        Ptr load(const std::string& logical, const ImageLoadOptions& opt);
        Ptr load(const std::string& logical);
        Ptr load(const fs::path& path, const ImageLoadOptions& opt);
        Ptr load(const fs::path& path);

        // Memory-backed entries (not hot-reloaded)
        Ptr loadFromMemory(const std::string& key, const unsigned char* bytes, int byte_count,
            const ImageLoadOptions& opt);
        Ptr loadFromMemory(const std::string& key, const unsigned char* bytes, int byte_count);

        // Batch ------------------------------------------------------------
        std::vector<Ptr> loadAll(const std::vector<std::string>& logicals, const ImageLoadOptions& opt);
        std::vector<Ptr> loadAll(const std::vector<std::string>& logicals);
        std::vector<Ptr> loadAll(const std::vector<fs::path>& paths, const ImageLoadOptions& opt);
        std::vector<Ptr> loadAll(const std::vector<fs::path>& paths);

        // Directory preload ------------------------------------------------
        int preloadDir(const fs::path& dir, const std::vector<std::string>& exts, bool recursive = true);

        // Reload / Housekeeping -------------------------------------------
        bool reload(const fs::path& path);
        bool reload(const std::string& logical);
        bool reload(const std::string& logical, const ImageLoadOptions& opt);
        int  reloadChanged();

        int  pruneUnused();
        bool unload(const std::string& logical); // remove a single cached entry
        bool unload(const fs::path& path);

        void clear();
        std::size_t size() const;

        struct DebugInfo
        {
            std::string key;      // normalized physical-key used in cache
            std::string label;    // original logical string (alias or path)
            std::string path;     // resolved physical path (string form)
            int w = 0, h = 0, refs = 0; bool exists = false; bool memory_backed = false;
        };
        std::vector<DebugInfo> snapshot() const;

#ifdef IMGUI_VERSION
        // Optional ImGui debug panel (implemented in .cpp)
        friend void DrawImageManagerPanel(ImageManager& mgr);
#endif

    private:
        using Clock = std::chrono::steady_clock;

        struct Record
        {
            Ptr obj;                       // shared Image object (stable handle)
            fs::path path;                 // resolved physical path (or label for memory assets)
            fs::file_time_type timestamp{};// last known file timestamp
            ImageLoadOptions options{};    // stored options per asset
            bool memory_backed = false;    // not file-based -> no hot reload
            Clock::time_point last_access{}; // for potential LRU policies
            std::string label;             // original logical identifier provided by caller
        };

        // Helpers implemented in .cpp
        std::pair<std::string, fs::path> resolveLogical_(const std::string& logical) const;
        static std::string normalize_(const fs::path& p);
        static fs::path canonDir_(const fs::path& p);
        static fs::file_time_type safeTimestamp_(const fs::path& p);
        static void loadInto_(Image& dst, const fs::path& p, const ImageLoadOptions& opt);
        static void loadIntoMemory_(Image& dst, const unsigned char* bytes, int byte_count, const ImageLoadOptions& opt);
        static bool isAliasSyntax_(const std::string& s);

        mutable std::mutex mtx_;
        std::unordered_map<std::string, Record> images_;                    // key: normalized physical path
        std::unordered_map<std::string, std::vector<fs::path>> aliases_;    // alias -> root directories
        ImageLoadOptions defaults_{}; // used by overloads without explicit options
    };

#ifdef IMGUI_VERSION
    void DrawImageManagerPanel(ImageManager& mgr);
#endif

} // namespace assets