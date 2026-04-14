#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

#ifndef DZEMIKK_ASSET_MANAGER_H
#define DZEMIKK_ASSET_MANAGER_H

#include "assetManager/iAssetHandler.h"
#include "core/iEngineModule.h"

#include <filesystem>
#include <fmod/include/fmod/fmod.hpp>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace dzemikk {

class Mesh;

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

    void Initialize() override;
    void UnInitialize() override;

#pragma endregion

#pragma region Public API

    template <typename T> AssetHandle<T> get(const std::string& path);

    template <typename T> AssetHandle<T> reload(const std::string& path);

    void unload(const std::string& path);

    enum class PrimitiveMesh : std::uint8_t { Cube, Quad, Sphere, Capsule };

    Mesh* getPrimitive(PrimitiveMesh type);

    // FOR TEST ONLY
    void setFMODSystem(FMOD::System* system);
    FMOD::System* getFMODSystem();

#pragma endregion

  private:
    struct AssetEntry {
        std::shared_ptr<void> handle;
        std::type_index type = typeid(void);
    };

    struct PrimitiveMeshHash {
        std::size_t operator()(PrimitiveMesh mesh) const {
            return std::hash<int>()(static_cast<int>(mesh));
        }
    };

    std::unordered_map<std::type_index, std::unique_ptr<IAssetHandlerBase>> _handlers;
    std::unordered_map<std::string, AssetEntry> _assets;
    std::unordered_map<PrimitiveMesh, std::unique_ptr<Mesh>, PrimitiveMeshHash> _builtinMeshes;

    FMOD::System* _system = nullptr;

#pragma region Internal

    void initPrimitiveMeshes();
    void registerHandlers();

    std::string resolvePath(const std::string& path);
    static std::optional<std::filesystem::path> findResRoot();

    std::unordered_map<std::string, std::string> _pathIndex;
    std::string _rootPath;

#pragma endregion
};

template <typename T> AssetHandle<T> AssetManager::get(const std::string& path) {
    auto it = _assets.find(path);

    if (it != _assets.end()) {
        if (it->second.type != typeid(T)) {
            throw std::runtime_error("Asset type mismatch: " + path);
        }

#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetManager] Loaded from cache: {}", path);
#endif

        auto shared = std::static_pointer_cast<T>(it->second.handle);
        return AssetHandle<T>(shared);
    }

    auto handlerIt = _handlers.find(typeid(T));
    if (handlerIt == _handlers.end())
        return {};

    auto* handler = static_cast<IAssetHandler<T>*>(handlerIt->second.get());

    auto result = handler->load(resolvePath(path));

    if (!result.isValid())
        return {};

    AssetEntry entry;
    entry.handle = result.resource;
    entry.type = typeid(T);

    _assets[path] = entry;

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetManager] Loaded from file: {}", path);
#endif

    return AssetHandle<T>(result.resource);
}

template <typename T> AssetHandle<T> AssetManager::reload(const std::string& path) {
    auto it = _assets.find(path);

    if (it == _assets.end())
        return get<T>(path);

    if (it->second.type != typeid(T)) {
        throw std::runtime_error("Asset type mismatch on reload: " + path);
    }

    auto handlerIt = _handlers.find(typeid(T));
    if (handlerIt == _handlers.end())
        return {};

    auto* handler = static_cast<IAssetHandler<T>*>(handlerIt->second.get());

    auto shared = std::static_pointer_cast<T>(it->second.handle);
    AssetHandle<T> handle(shared);

    handler->reload(handle, resolvePath(path));

    return handle;
}

} // namespace dzemikk

#endif // DZEMIKK_ASSET_MANAGER_H