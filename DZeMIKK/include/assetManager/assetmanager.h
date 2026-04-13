#ifndef DZEMIKK_ASSET_MANAGER_H
#define DZEMIKK_ASSET_MANAGER_H

#include "core/iEngineModule.h"
#include "assetManager/iAssetHandler.h"

#include <string>
#include <typeindex>
#include <unordered_map>
#include <stdexcept>
#include <optional>
#include <filesystem>

#include<fmod/include/fmod/fmod.hpp>

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

    template <typename T> T* get(const std::string& path);
    template <typename T> T* reload(const std::string& path);
    void unload(const std::string& path);

    enum class PrimitiveMesh : std::uint8_t { Cube, Quad, Sphere, Capsule };

    dzemikk::Mesh* getPrimitive(PrimitiveMesh type);

    //FOR TEST ONLY - DELETE THIS
    void setFMODSystem(FMOD::System* system);
    FMOD::System* getFMODSystem();

#pragma endregion

  private:
    struct AssetEntry {
        void* data = nullptr;
        std::type_index type = typeid(void);
    };

    struct PrimitiveMeshHash {
        std::size_t operator()(PrimitiveMesh mesh) const {
            return std::hash<int>()(static_cast<int>(mesh));
        }
    };

    std::unordered_map<std::type_index, std::unique_ptr<IAssetHandler>> _handlers;
    std::unordered_map<std::string, AssetEntry> _assets;
    std::unordered_map<PrimitiveMesh, std::unique_ptr<dzemikk::Mesh>, PrimitiveMeshHash> _builtinMeshes;

    // FOR TEST ONLY - DELETE THIS
    FMOD::System* _system = nullptr;

#pragma region Internal

    void initPrimitiveMeshes();

    void registerHandlers();

    void* loadInternal(const std::string& path, std::type_index type);

    void reloadInternal(const std::string& path, void* data, std::type_index type);

    std::string resolvePath(const std::string& path);
    static std::optional<std::filesystem::path> findResRoot();

    std::unordered_map<std::string, std::string> _pathIndex;
    std::string _rootPath;

#pragma endregion
};

template <typename T> inline T* AssetManager::get(const std::string& path) {
    auto assetIt = _assets.find(path);

    if (assetIt != _assets.end()) {
        if (assetIt->second.type != std::type_index(typeid(T))) {
            throw std::runtime_error("Asset type mismatch for id: " + path);
        }

        return static_cast<T*>(assetIt->second.data);
    }

    void* rawData = loadInternal(path, std::type_index(typeid(T)));

    if (!rawData) {
            return nullptr;
    }

    AssetEntry entry;
    entry.data = rawData;
    entry.type = std::type_index(typeid(T));

    _assets[path] = entry;

    return static_cast<T*>(rawData);
}

template <typename T> inline T* AssetManager::reload(const std::string& path) {
    auto assetIt = _assets.find(path);

    if (assetIt == _assets.end()) {
        return get<T>(path);
    }

    if (assetIt->second.type != std::type_index(typeid(T))) {
        throw std::runtime_error("Asset type mismatch on reload: " + path);
    }

    T* asset = static_cast<T*>(assetIt->second.data);

    reloadInternal(path, asset, std::type_index(typeid(T)));

    return asset;
}

} // namespace dzemikk

#endif // DZEMIKK_ASSET_MANAGER_H