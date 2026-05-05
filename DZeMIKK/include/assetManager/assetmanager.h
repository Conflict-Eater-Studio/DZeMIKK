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
#include "assetManager/threadPool.h"

#include "renderer/model.h"
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
     * @brief Task wrapper for async asset loading with user context.
     *
     * Used by callback-based async API to pass additional user data
     * alongside the asset loading result.
     */
    template <typename T, typename Context> struct AssetTask {
        Context context;
        std::function<void(AssetHandle<T>, Context&)> onLoad;
    };

    /**
     * @brief Synchronously loads an asset.
     *
     * @note Blocks if asset is not yet loaded.
     * @note May reuse in-flight async load if already running.
     */
    template <typename T> 
    AssetHandle<T> get(const std::string& path);

    /**
     * @brief Asynchronously loads an asset (future-based API).
     *
     * @return shared_future resolving to AssetHandle when ready.
     * @note Reuses cached or in-flight loads if possible.
     */
    template <typename T> 
    std::shared_future<AssetHandle<T>> getAsync(const std::string& path);

    /**
     * @brief Asynchronous load with callback + user context.
     *
     * @note Executes callback when asset becomes available.
     * @note If asset is already loaded, callback is invoked immediately.
     */
    template <typename T, typename Context>
    void getAsync(const std::string& path, AssetTask<T, Context> assetTask);

    /**
     * @brief Reloads an already loaded asset from source.
     *
     * @note Keeps handle valid, updates underlying resource.
     */
    template <typename T> 
    AssetHandle<T> reload(const std::string& path);

    /**
     * @brief Removes asset from cache.
     *
     * @note Asset will be destroyed if no external references exist.
     */
    void unload(const std::string& path);

    /**
     * @brief Returns built-in primitive model.
     *
     * @note Cached asset, does not load from disk.
     */
    AssetHandle<Model> getPrimitiveModel(PrimitiveMeshLibrary::PrimitiveMesh type);

    /**
     * @brief Returns raw pointer to built-in primitive mesh.
     *
     * @warning Non-owning pointer, valid as long as AssetManager exists.
     * @note Prefer getPrimitiveModel() when possible.
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

    /**
     * @brief Processes pending GPU upload queue.
     *
     * @note Should be called on main/render thread.
     * @note Uploads resources marked as GPU uploadable.
     */
    void processGpuUploads();

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

    /** @brief Worker thread pool for async asset loading */
    ThreadPool _threadPool;

    /** @brief Queue of GPU-uploadable resources awaiting main thread upload */
    std::queue<std::shared_ptr<IGpuUploadable>> _gpuUploadQueue;

    /** @brief Protects GPU upload queue access */
    std::mutex _gpuMutex;

    /**
     * @brief Tracks currently loading assets (in-flight async operations).
     *
     * Key: asset path
     * Value: shared future of loaded asset
     *
     * @note Prevents duplicate loading of the same asset.
     * @note Shared between sync and async API.
     */
    std::unordered_map<std::string, std::shared_future<std::shared_ptr<void>>> _inFlight;

    /** @brief Protects access to in-flight loading map. */
    std::mutex _inFlightMutex;

    /**
     * @brief Internal async asset loading execution.
     *
     * Runs on worker thread, loads asset and resolves promise.
     */
    template <typename T>
    void executeAssetLoad(const std::string& path,
                          const std::shared_ptr<std::promise<std::shared_ptr<void>>>& promise);

    /**
     * @brief Registers all asset loaders in the registry.
     *
     * This function initializes and binds concrete AssetHandler implementations
     * to their corresponding asset types (e.g. textures, meshes, audio).
     * It is typically called during AssetManager::Initialize().
     */
    void registerHandlers();

};

// ================================== IMPLEMENTATION ==================================
template <typename T> AssetHandle<T> AssetManager::get(const std::string& path) {
    if (auto *entry = _database.getEntry(path);
        entry && entry->state == AssetDatabase::AssetState::Ready) {

#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetManager] Cache hit: {}", path);
#endif

        return AssetHandle<T>(std::static_pointer_cast<T>(entry->handle), path);
    }

    {
        std::lock_guard lock(_inFlightMutex);

        auto it = _inFlight.find(path);
        if (it != _inFlight.end()) {

#if DZEMIKK_DEV_TOOLS
            spdlog::info("[AssetManager] Waiting for async (future): {}", path);
#endif

            auto future = it->second;

            auto ptr = std::static_pointer_cast<T>(future.get());

            if (!ptr) {
                return {};
            }

            return AssetHandle<T>(ptr, path);
        }
    }

    auto* handler = _loaders.get<T>();
    if (!handler) {
        return {};
    }

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetManager] Loading sync: {}", path);
#endif
    _database.insertLoading<T>(path);

    auto result =
        handler->load(_resources.resolve(path), IAssetHandlerBase::LoadExecutionMode::Sync);

    if (!result.isValid()) {
        _database.setFailed(path);
        return {};
    }

    auto ptr = std::static_pointer_cast<T>(result.resource);

    _database.setReady<T>(path, ptr);

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetManager] Loaded sync: {}", path);
#endif

    return AssetHandle<T>(ptr, path);
}

template <typename T> std::shared_future<AssetHandle<T>> AssetManager::getAsync(const std::string& path) {
    if (auto *entry = _database.getEntry(path);
        entry && entry->state == AssetDatabase::AssetState::Ready) {

#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetManager] Cache hit (READY): {}", path);
#endif

        auto cached = std::static_pointer_cast<T>(entry->handle);

        return std::async(std::launch::deferred,
                          [cached, path]() { return AssetHandle<T>(cached, path); })
            .share();
    }

    {
        std::lock_guard lock(_inFlightMutex);

        auto it = _inFlight.find(path);
        if (it != _inFlight.end()) {

#if DZEMIKK_DEV_TOOLS
            spdlog::info("[AssetManager] Reusing in-flight: {}", path);
#endif

            auto future = it->second;

            auto wrapperPromise = std::make_shared<std::promise<AssetHandle<T>>>();
            auto wrapperFuture = wrapperPromise->get_future().share();

            _threadPool.enqueue({path, [future, wrapperPromise, path]() {
                                     auto ptr = std::static_pointer_cast<T>(future.get());
                                     wrapperPromise->set_value(AssetHandle<T>(ptr, path));
                                 }});

            return wrapperFuture;
        }
    }

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetManager] Enqueue async load: {}", path);
#endif

    auto* handler = _loaders.get<T>();
    if (!handler) {
        return {};
    }

    auto promise = std::make_shared<std::promise<std::shared_ptr<void>>>();
    auto future = promise->get_future().share();

    {
        std::lock_guard lock(_inFlightMutex);
        _inFlight[path] = future;
    }

    _database.insertLoading<T>(path);

    ThreadPool::AssetJob job;
    job.path = path;

    job.execute = [this, path, promise]() { executeAssetLoad<T>(path, promise); };

    _threadPool.enqueue(std::move(job));

    auto wrapperPromise = std::make_shared<std::promise<AssetHandle<T>>>();
    auto wrapperFuture = wrapperPromise->get_future().share();

    _threadPool.enqueue({path, [future, wrapperPromise, path]() {
                             auto ptr = std::static_pointer_cast<T>(future.get());
                             wrapperPromise->set_value(AssetHandle<T>(ptr, path));
                         }});

    return wrapperFuture;
}

template <typename T> void AssetManager::executeAssetLoad(const std::string& path,
                                    const std::shared_ptr<std::promise<std::shared_ptr<void>>>& promise) {
    auto* handler = _loaders.get<T>();
    if (!handler) {
        promise->set_value(nullptr);
        return;
    }

    auto result =
        handler->load(_resources.resolve(path), IAssetHandlerBase::LoadExecutionMode::Async);

    if (!result.isValid()) {
        _database.setFailed(path);
        promise->set_value(nullptr);
        return;
    }

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetManager] Loaded in worker thread: {}", path);
#endif

    _database.setReady<T>(path, result.resource);

    if (auto gpu = std::dynamic_pointer_cast<IGpuUploadable>(result.resource)) {
        std::lock_guard lock(_gpuMutex);
        _gpuUploadQueue.push(gpu);
    }

    promise->set_value(result.resource);

    {
        std::lock_guard lock(_inFlightMutex);
        _inFlight.erase(path);
    }
}

template <typename T, typename Context> void AssetManager::getAsync(const std::string& path, AssetTask<T, Context> assetTask) {
    if (auto *entry = _database.getEntry(path);
        entry && entry->state == AssetDatabase::AssetState::Ready) {

#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetManager] Async (immediate ready): {}", path);
#endif

        auto ptr = std::static_pointer_cast<T>(entry->handle);

        assetTask.onLoad(AssetHandle<T>(ptr, path), assetTask.context);
        return;
    }

    {
        std::lock_guard lock(_inFlightMutex);

        auto it = _inFlight.find(path);
        if (it != _inFlight.end()) {

#if DZEMIKK_DEV_TOOLS
            spdlog::info("[AssetManager] Async (waiting in-flight): {}", path);
#endif

            auto future = it->second;

            _threadPool.enqueue(
                {path, [this, future, path, task = std::move(assetTask)]() mutable {
                     auto ptr = std::static_pointer_cast<T>(future.get());
                     task.onLoad(AssetHandle<T>(ptr, path), task.context);
                 }});

            return;
        }
    }

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetManager] Async (new load + callback): {}", path);
#endif

    auto* handler = _loaders.get<T>();
    if (!handler) {
        return;
    }

    auto promise = std::make_shared<std::promise<std::shared_ptr<void>>>();
    auto future = promise->get_future().share();

    {
        std::lock_guard lock(_inFlightMutex);
        _inFlight[path] = future;
    }

    _database.insertLoading<T>(path);

    ThreadPool::AssetJob job;
    job.path = path;

    job.execute = [this, path, promise, task = std::move(assetTask)]() mutable {
        executeAssetLoad<T>(path, promise);

        auto *entry = _database.getEntry(path);
        if (entry && entry->state == AssetDatabase::AssetState::Ready) {
            auto ptr = std::static_pointer_cast<T>(entry->handle);
            task.onLoad(AssetHandle<T>(ptr, path), task.context);
        }
    };

    _threadPool.enqueue(std::move(job));
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