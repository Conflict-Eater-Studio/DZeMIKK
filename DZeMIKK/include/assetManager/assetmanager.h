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
#include "assetManager/assetHandle.h"

#include "renderer/model.h"
#include "audio/sound.h"
#include <future>
#include <queue>
#include <functional>

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

    template <typename T> std::shared_future<AssetHandle<T>> getAsync(const std::string& path);
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
    AssetHandle<Model> getPrimitiveModel(PrimitiveMeshLibrary::PrimitiveMesh type);

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

    std::queue<std::shared_ptr<Model>> _gpuUploadQueue;
    std::mutex _gpuMutex;

    void processGpuUploads() {
        std::lock_guard lock(_gpuMutex);

        while (!_gpuUploadQueue.empty()) {
            auto model = _gpuUploadQueue.front();
            _gpuUploadQueue.pop();

            if (model)
                model->uploadToGPU();
        }
    }

#pragma endregion

  private:
    std::unordered_map<std::string, std::shared_future<std::shared_ptr<void>>> _inFlight;
    std::mutex _inFlightMutex;

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

    auto entry = _database.getEntry(path);

    if (entry) {

        if (entry->state == AssetDatabase::AssetState::Ready) {
#if DZEMIKK_DEV_TOOLS
            spdlog::info("[AssetManager] Loaded from cache: {}", path);
#endif
            return AssetHandle<T>(std::static_pointer_cast<T>(entry->handle), path);
        }

        if (entry->state == AssetDatabase::AssetState::Loading) {
#if DZEMIKK_DEV_TOOLS
            spdlog::info("[AssetManager] Waiting for async asset: {}", path);
#endif
            auto ptr = std::static_pointer_cast<T>(entry->future.get());

            return AssetHandle<T>(ptr, path);
        }

        return {};
    }

    auto* handler = _loaders.get<T>();
    if (!handler) {
        return {};
    }

    auto future =
        std::async(std::launch::deferred, [this, path, handler]() -> std::shared_ptr<void> {
            auto result =
                handler->load(_resources.resolve(path), IAssetHandlerBase::LoadExecutionMode::Sync);

            if (!result.isValid())
                return nullptr;

            return result.resource;
        }).share();

    _database.insertLoading<T>(path, future);

    auto ptr = std::static_pointer_cast<T>(future.get());

    if (!ptr) {
        _database.setFailed(path);
        return {};
    }

    _database.setReady(path, ptr);

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetManager] Loaded from file: {}", path);
#endif

    return AssetHandle<T>(ptr, path);
}

template <typename T>
std::shared_future<AssetHandle<T>> AssetManager::getAsync(const std::string& path) {

    auto entry = _database.getEntry(path);

    if (entry && entry->state == AssetDatabase::AssetState::Ready) {
#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetManager] Asset ready while LOADING: {}", path);
#endif
        auto cached = std::static_pointer_cast<T>(entry->handle);

        return std::async(std::launch::deferred,
                          [cached, path]() { return AssetHandle<T>(cached, path); })
            .share();
    }

    if (entry && entry->state == AssetDatabase::AssetState::Loading) {
#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetManager] Asset requested while LOADING: {}", path);
#endif

        return std::async(std::launch::deferred,
                          [entry, path]() {
                              auto ptr = std::static_pointer_cast<T>(entry->future.get());
#if DZEMIKK_DEV_TOOLS
                              spdlog::info("[AssetManager] Finished waiting for: {}", path);
#endif


                              return AssetHandle<T>(ptr, path);
                          })
            .share();
    }

    auto* handler = _loaders.get<T>();
    if (!handler) {
        return {};
    }

    auto future = std::async(std::launch::async, [this, path, handler]() -> std::shared_ptr<void> {
                      auto result = handler->load(_resources.resolve(path),
                                                  IAssetHandlerBase::LoadExecutionMode::Async);

                      if (!result.isValid())
                          return nullptr;

                      _database.setReady<T>(path, result.resource); 

                      // TEST -> FOR MODEL ONLY FIX THIS
                      {
                          std::lock_guard lock(_gpuMutex);
                          _gpuUploadQueue.push(std::static_pointer_cast<Model>(result.resource));
                      }

                      return result.resource;
                  }).share();

    _database.insertLoading<T>(path, future);

    return std::async(std::launch::deferred,
                      [future, path]() {
                          auto ptr = std::static_pointer_cast<T>(future.get());
                          return AssetHandle<T>(ptr, path);
                      })
        .share();
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