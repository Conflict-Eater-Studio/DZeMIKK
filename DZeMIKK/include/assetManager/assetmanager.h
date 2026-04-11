#ifndef DZEMIKK_ASSET_MANAGER_H
#define DZEMIKK_ASSET_MANAGER_H

#include "core/iEngineModule.h"

#include <string>
#include <typeindex>
#include <unordered_map>
#include <stdexcept>
#include <optional>
#include <filesystem>

namespace dzemikk {
class Mesh;
class Shader; 
class Font;
class Skybox;
class Texture;

class AssetManager : public IEngineModule {
  public:
    AssetManager() = default;
    ~AssetManager() = default;

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

    template <typename T> T* Get(const std::string& id);
    void Unload(const std::string& id);

    enum class PrimitiveMesh { Cube, Quad, Sphere, Capsule };

    Mesh* GetPrimitive(PrimitiveMesh type);

#pragma endregion

  private:
    struct AssetEntry {
        void* data = nullptr;
        std::type_index type = typeid(void);
    };

    struct PrimitiveMeshHash {
        std::size_t operator()(PrimitiveMesh m) const {
            return std::hash<int>()(static_cast<int>(m));
        }
    };

    std::unordered_map<std::string, AssetEntry> _assets;
    std::unordered_map<PrimitiveMesh, Mesh*, PrimitiveMeshHash> _builtinMeshes;

#pragma region Internal

    void initPrimitiveMeshes();
    Mesh* createCubeMesh();
    Mesh* createQuadMesh();
    Mesh* createSphereMesh();
    Mesh* createCapsuleMesh();

    void* LoadInternal(const std::string& id, std::type_index type);
    Mesh* loadMeshFromFile(const std::string& path);
    dzemikk::Texture* loadTextureFromFile(const std::string& path, bool flipVertical = true);
    dzemikk::Shader* loadShaderFromFile(const std::string& path);
    dzemikk::Font* loadFontFromFile(const std::string& path);
    dzemikk::Skybox* loadSkyboxFromFile(const std::string& basePath);

    std::string resolvePath(const std::string& id);
    std::optional<std::filesystem::path> findResRoot();

    std::unordered_map<std::string, std::string> _pathIndex;
    std::string _rootPath;

#pragma endregion
};

template <typename T> inline T* AssetManager::Get(const std::string& id) {
    auto it = _assets.find(id);

    if (it != _assets.end()) {
        if (it->second.type != std::type_index(typeid(T))) {
            throw std::runtime_error("Asset type mismatch for id: " + id);
        }

        return static_cast<T*>(it->second.data);
    }

    void* rawData = LoadInternal(id, std::type_index(typeid(T)));

    if (!rawData)
        return nullptr;

    AssetEntry entry;
    entry.data = rawData;
    entry.type = std::type_index(typeid(T));

    _assets[id] = entry;

    return static_cast<T*>(rawData);
}

} // namespace dzemikk

#endif // DZEMIKK_ASSET_MANAGER_H