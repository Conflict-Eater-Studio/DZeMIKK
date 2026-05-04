#ifndef DZEMIKK_ASSET_DATABASE_H
#define DZEMIKK_ASSET_DATABASE_H

#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <future>
#include <mutex>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

namespace dzemikk {

/**
 * @brief Central cache for loaded assets.
 *
 * Stores assets indexed by their path and ensures type safety
 * during retrieval using runtime type information.
 */
class AssetDatabase {
  public:
    AssetDatabase() = default;
    ~AssetDatabase() = default;

    AssetDatabase(const AssetDatabase&) = delete;
    AssetDatabase& operator=(const AssetDatabase&) = delete;
    AssetDatabase(AssetDatabase&&) noexcept = delete;
    AssetDatabase& operator=(AssetDatabase&&) noexcept = delete;

    /**
     * @brief Retrieves an asset of type T.
     *
     * @tparam T Expected asset type.
     * @param path Asset path key.
     * @return std::shared_ptr<T> Asset pointer or nullptr if not found.
     * @throws std::runtime_error if type mismatch occurs.
     */
    template <typename T> 
    std::shared_ptr<T> get(const std::string& path) const {
        std::lock_guard lock(_mutex);
        auto it = _assets.find(path);
        if (it == _assets.end()) {
            return nullptr;
        }

        if (it->second.type != typeid(T)) {
            throw std::runtime_error("Asset type mismatch: " + path);
        }

#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetDatabase] Cache hit: {}", path);
#endif

        return std::static_pointer_cast<T>(it->second.handle);
    }

    /**
     * @brief Stores an asset in the database.
     *
     * @tparam T Asset type.
     * @param path Asset path key.
     * @param asset Shared pointer to asset.
     */
    template <typename T> void store(const std::string& path, std::shared_ptr<T> asset) {
        std::lock_guard lock(_mutex);
        if (!asset) {
            return;
        }

        Entry entry;
        entry.handle = asset;
        entry.type = typeid(T);

        _assets[path] = std::move(entry);

#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetDatabase] Stored: {}", path);
#endif
    }

    /**
     * @brief Removes an asset from the database.
     */
    void remove(const std::string& path);
    
    /**
     * @brief Clears the entire asset cache.
     */
    void clear();

    
    /**
     * @brief Returns the stored type of an asset.
     *
     * @throws std::runtime_error if asset does not exist.
     */
    [[nodiscard]] std::type_index getType(const std::string& path) const;
    
    /**
     * @brief Retrieves raw asset pointer (type-erased).
     */
    [[nodiscard]] std::shared_ptr<void> getRaw(const std::string& path) const;

    /**
     * @brief Checks if asset exists.
     */
    [[nodiscard]] bool contains(const std::string& path) const;

    enum class AssetState { Loading, Ready, Failed };

    /**
     * @brief Internal representation of a stored asset.
     *
     * Uses type erasure (std::shared_ptr<void>) to allow storing assets of
     * different types in a single container. The actual type information
     * is preserved separately using std::type_index.
     */
    struct Entry {
        std::shared_ptr<void> handle;
        std::type_index type = typeid(void);
        AssetState state = AssetState::Loading;
    };

    template <typename T> void setReady(const std::string& path, std::shared_ptr<T> asset) {
        std::lock_guard lock(_mutex);
        auto it = _assets.find(path);
        if (it == _assets.end())
            return;

        it->second.handle = asset;
        it->second.type = typeid(T);
        it->second.state = AssetState::Ready;

#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetDatabase] Set READY: {}", path);
#endif
    }

    template <typename T> void insertLoading(const std::string& path) {
        std::lock_guard lock(_mutex);
        Entry entry;
        entry.type = typeid(T);
        entry.state = AssetState::Loading;

        _assets[path] = std::move(entry);

#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetDatabase] Insert LOADING: {}", path);
#endif
    }

    void setFailed(const std::string& path) {
        std::lock_guard lock(_mutex);
        auto it = _assets.find(path);
        if (it == _assets.end()) {
            return;
        }

        it->second.state = AssetState::Failed;
        it->second.handle.reset();

#if DZEMIKK_DEV_TOOLS
        spdlog::error("[AssetDatabase] Set FAILED: {}", path);
#endif
    }

    Entry* getEntry(const std::string& path) {
        std::lock_guard lock(_mutex);
        auto it = _assets.find(path);
        if (it == _assets.end())
            return nullptr;

        return &it->second;
    }

  private:
    
    /**
     * @brief Asset storage container.
     *
     * Maps asset paths (keys) to their corresponding entries.
     * Provides fast lookup (O(1) average) for asset retrieval.
     */
    std::unordered_map<std::string, Entry> _assets;
    mutable std::mutex _mutex;
};

} // namespace dzemikk
#endif // DZEMIKK_ASSET_DATABASE_H