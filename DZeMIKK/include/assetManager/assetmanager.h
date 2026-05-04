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

    template <typename T, typename Context> struct AssetTask {
        Context context;
        std::function<void(AssetHandle<T>, Context&)> onLoad;
    };

    template <typename T, typename Context>
    void getAsync(const std::string& path, AssetTask<T, Context> assetTask);


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

    ThreadPool _threadPool;

    template <typename T>
    void executeAssetLoad(const std::string& path,
                          std::shared_ptr<std::promise<std::shared_ptr<void>>> promise);

#pragma region Internal

    /**
     * @brief Registers all asset loaders in the registry.
     *
     * This function initializes and binds concrete AssetHandler implementations
     * to their corresponding asset types (e.g. textures, meshes, audio).
     * It is typically called during AssetManager::Initialize().
     */
    void registerHandlers();

    std::unordered_map<std::string, std::shared_future<std::shared_ptr<void>>> _inFlight;
    std::mutex _inFlightMutex;

#pragma endregion
};

// ================= IMPLEMENTATION =================

template <typename T> AssetHandle<T> AssetManager::get(const std::string& path) {
    if (auto entry = _database.getEntry(path);
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

            if (!ptr)
                return {};

            return AssetHandle<T>(ptr, path);
        }
    }

    auto* handler = _loaders.get<T>();
    if (!handler)
        return {};

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

    if constexpr (std::is_same_v<T, Model>) {
        std::lock_guard lock(_gpuMutex);
        _gpuUploadQueue.push(result.resource);
    }

    return AssetHandle<T>(ptr, path);
}

template <typename T>
std::shared_future<AssetHandle<T>> AssetManager::getAsync(const std::string& path) {
    if (auto entry = _database.getEntry(path);
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
    if (!handler)
        return {};

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

template <typename T>
void AssetManager::executeAssetLoad(const std::string& path,
                                    std::shared_ptr<std::promise<std::shared_ptr<void>>> promise) {
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

    if constexpr (std::is_same_v<T, Model>) {
        std::lock_guard lock(_gpuMutex);
        _gpuUploadQueue.push(result.resource);
    }

    promise->set_value(result.resource);

    {
        std::lock_guard lock(_inFlightMutex);
        _inFlight.erase(path);
    }
}

template <typename T, typename Context>
void AssetManager::getAsync(const std::string& path, AssetTask<T, Context> assetTask) {
    // ---------------- READY ----------------
    if (auto entry = _database.getEntry(path);
        entry && entry->state == AssetDatabase::AssetState::Ready) {

#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetManager] Async (immediate ready): {}", path);
#endif

        auto ptr = std::static_pointer_cast<T>(entry->handle);

        assetTask.onLoad(AssetHandle<T>(ptr, path), assetTask.context);
        return;
    }

    // ---------------- IN-FLIGHT ----------------
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

    // ---------------- START NEW LOAD ----------------
#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetManager] Async (new load + callback): {}", path);
#endif

    auto* handler = _loaders.get<T>();
    if (!handler)
        return;

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

        auto entry = _database.getEntry(path);
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