#ifndef DZEMIKK_ASSET_MANAGER_H
#define DZEMIKK_ASSET_MANAGER_H

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

#include "core/iEngineModule.h"

#include <fmod/include/fmod/fmod.hpp>
#include <stdexcept>
#include <string>

#include "assetManager/assetDatabase.h"
#include "assetManager/assetLoaderRegistry.h"
#include "assetManager/resourceIndex.h"
#include "assetManager/primitiveMeshLibrary.h"

namespace dzemikk {

class Mesh;

/**
 * @brief Central asset management system.
 *
 * Responsible for:
 * - Loading assets via registered handlers
 * - Caching assets in AssetDatabase
 * - Providing lightweight handles (AssetHandle)
 * - Managing reload/unload lifecycle
 */
class AssetManager : public IEngineModule {
  public:
    AssetManager() = default;
    ~AssetManager() override = default;

#pragma region Disable copy/move

    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&) noexcept = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager& operator=(AssetManager&&) noexcept = delete;

#pragma endregion

#pragma region Initialization / Uninitialization

    void initialize() override;
    void uninitialize() override;

#pragma endregion

#pragma region Public API

    /**
     * @brief Gets an asset handle. Loads it if not cached.
     */
    template <typename T> 
    AssetHandle<T> get(const std::string& path);

    /**
     * @brief Reloads an already loaded asset.
     */
    template <typename T> 
    AssetHandle<T> reload(const std::string& path);

    /**
     * @brief Unloads asset from cache.
     */
    void unload(const std::string& path);

    /**
     * @brief Returns built-in primitive mesh.
     */
    Mesh* getPrimitive(PrimitiveMeshLibrary::PrimitiveMesh type);

    // FOR TEST ONLY
    /**
     * @brief Sets FMOD system (temporary / testing).
     */
    void setFMODSystem(FMOD::System* system);

    /**
     * @brief Gets FMOD system.
     */
    [[nodiscard]] FMOD::System* getFMODSystem() const;

#pragma endregion

  private:
    /** @brief Asset cache */
    AssetDatabase _database;

    /** @brief Loader registry */
    AssetLoaderRegistry _loaders;

    /** @brief Resource path resolver */
    ResourceIndex _resources;
      
    /** @brief Built-in primitives */
    PrimitiveMeshLibrary _primitiveMeshLibrary;

    /** @brief External audio system (non-owning) */
    FMOD::System* _system = nullptr;

#pragma region Internal

    /**
     * @brief Registers all asset loaders in the registry.
     *
     * This function initializes and binds concrete AssetHandler implementations
     * to their corresponding asset types (e.g. textures, meshes, audio).
     * It is typically called during AssetManager::Initialize().
     */
    void registerHandlers();

#pragma endregion
};

// ================= IMPLEMENTATION =================

template <typename T> AssetHandle<T> AssetManager::get(const std::string& path) {
    if (auto cached = _database.get<T>(path)) {
#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetManager] Loaded from cache: {}", path);
#endif
        return AssetHandle<T>(cached, path);
    }

    auto* handler = _loaders.get<T>();
    if (!handler) {
        return {};
    }

    auto result = handler->load(_resources.resolve(path));
    if (!result.isValid()) {
        return {};
    }

    _database.store<T>(path, result.resource);

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetManager] Loaded from file: {}", path);
#endif

    return AssetHandle<T>(result.resource, path);
}

template <typename T> AssetHandle<T> AssetManager::reload(const std::string& path) {

    auto shared = _database.get<T>(path);

    if (!shared) {
        return get<T>(path);
    }

    auto* handler = _loaders.get<T>();
    if (!handler) {
        return {};
    }

    AssetHandle<T> handle(shared, path);

    if (!handler->reload(handle, _resources.resolve(path))) {
        return {};
    }

    return handle;
}
} // namespace dzemikk

#endif // DZEMIKK_ASSET_MANAGER_H