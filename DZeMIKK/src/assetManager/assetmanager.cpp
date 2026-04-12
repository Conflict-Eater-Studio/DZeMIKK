#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

#include "assetManager/assetmanager.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include "renderer/font.h"
#include "renderer/skybox.h"
#include "renderer/texture.h"
#include "audio/sound.h"

#include "assetManager/fontHandler.h"
#include "assetManager/meshHandler.h"
#include "assetManager/shaderHandler.h"
#include "assetManager/skyboxHandler.h"
#include "assetManager/soundHandler.h"
#include "assetManager/textureHandler.h"

#include "assetManager/primitiveFactory.h"

#include <iostream>
#include <stb/stb_image.h>
#include <fstream>
#include <iterator>
#include <corecrt_math_defines.h>
#include <assimp/version.h>

void dzemikk::AssetManager::Initialize() {
#if DZEMIKK_DEV_TOOLS
    auto t0 = std::chrono::high_resolution_clock::now();
    spdlog::info("[AssetManager] Initialization started");

    spdlog::info("Assimp version: {}.{}.{}", aiGetVersionMajor(), aiGetVersionMinor(),
                 aiGetVersionRevision());
#endif

    _pathIndex.clear();

    auto rootOpt = findResRoot();
    if (!rootOpt) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("[AssetManager] Cannot find 'res' folder!");
#else
        std::cerr << "[AssetManager] ERROR: cannot find 'res' folder!\n";
#endif
        return;
    }

    _rootPath = rootOpt->string();
    std::replace(_rootPath.begin(), _rootPath.end(), '\\', '/');

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetManager] Resource root: {}", _rootPath);
#endif

    size_t fileCount = 0;

    for (auto& p : std::filesystem::recursive_directory_iterator(_rootPath)) {
        if (!p.is_regular_file())
            continue;

        std::string fullPath = p.path().string();
        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

        std::string relative = std::filesystem::relative(p.path(), _rootPath).string();
        std::replace(relative.begin(), relative.end(), '\\', '/');

        _pathIndex[relative] = fullPath;
        ++fileCount;
    }
    RegisterHandlers();
    initPrimitiveMeshes();

#if DZEMIKK_DEV_TOOLS
    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    spdlog::info("[AssetManager] Initialization finished in {} ms", ms);
#endif
}

std::optional<std::filesystem::path> dzemikk::AssetManager::findResRoot() {
    namespace fs = std::filesystem;

    fs::path start = fs::current_path();

    for (auto& p : fs::recursive_directory_iterator(start)) {
        if (p.is_directory() && p.path().filename() == "res") {
            return fs::absolute(p.path());
        }
    }

    return std::nullopt;
}

void dzemikk::AssetManager::UnInitialize() {
    for (auto& [id, entry] : _assets) {
        auto handlerIt = _handlers.find(entry.type);
        if (handlerIt != _handlers.end()) {
            handlerIt->second->unload(entry.data);
        }
    }

    _assets.clear();
}

void* dzemikk::AssetManager::loadInternal(const std::string& id, std::type_index type) {
    auto it = _handlers.find(type);
    if (it == _handlers.end())
        return nullptr;

    std::string path = resolvePath(id); 

    return it->second->load(path);
}

std::string dzemikk::AssetManager::resolvePath(const std::string& id) {
    auto it = _pathIndex.find(id);
    if (it != _pathIndex.end())
        return it->second;

    return _rootPath + "/" + id;
}

void dzemikk::AssetManager::Unload(const std::string& id) {
    auto it = _assets.find(id);
    if (it == _assets.end())
        return;

    AssetEntry& entry = it->second;

    auto handlerIt = _handlers.find(entry.type);
    if (handlerIt != _handlers.end()) {
        handlerIt->second->unload(entry.data);
    }

    _assets.erase(it);
}

void dzemikk::AssetManager::initPrimitiveMeshes() {
    _builtinMeshes[PrimitiveMesh::Cube] = PrimitiveFactory::CreateCube();
    _builtinMeshes[PrimitiveMesh::Quad] = PrimitiveFactory::CreateQuad();
    _builtinMeshes[PrimitiveMesh::Sphere] = PrimitiveFactory::CreateSphere();
    _builtinMeshes[PrimitiveMesh::Capsule] = PrimitiveFactory::CreateCapsule();
}

dzemikk::Mesh* dzemikk::AssetManager::GetPrimitive(PrimitiveMesh type) {
    auto it = _builtinMeshes.find(type);
    if (it != _builtinMeshes.end())
        return it->second;

    return nullptr;
}

void dzemikk::AssetManager::setFMODSystem(FMOD::System* system) {
    this->system = system;

    auto it = _handlers.find(typeid(Sound));
    auto* handler = static_cast<SoundHandler*>(it->second.get());
    handler->system = system;
}

void dzemikk::AssetManager::RegisterHandlers() {
    _handlers[typeid(Mesh)] = std::make_unique<MeshHandler>();
    _handlers[typeid(Shader)] = std::make_unique<ShaderHandler>();
    _handlers[typeid(Texture)] = std::make_unique<TextureHandler>();
    _handlers[typeid(Skybox)] = std::make_unique<SkyboxHandler>();
    _handlers[typeid(Font)] = std::make_unique<FontHandler>();
    _handlers[typeid(Sound)] = std::make_unique<SoundHandler>();
}

void dzemikk::AssetManager::reloadInternal(const std::string& path, void* data, std::type_index type) {
    auto it = _handlers.find(type);
    if (it == _handlers.end())
        return;

    it->second->reload(data, resolvePath(path));
}