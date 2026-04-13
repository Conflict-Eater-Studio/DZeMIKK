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

#include <algorithm>
#include <ranges>
#include <iostream>
#include <stb/stb_image.h>
#include <fstream>
#include <iterator>
#include <corecrt_math_defines.h>
#include <assimp/version.h>

void dzemikk::AssetManager::Initialize() {
#if DZEMIKK_DEV_TOOLS
    auto timer0 = std::chrono::high_resolution_clock::now();
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
    std::ranges::replace(_rootPath, '\\', '/');

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetManager] Resource root: {}", _rootPath);
#endif

    size_t fileCount = 0;

    auto dirIt = std::filesystem::recursive_directory_iterator(_rootPath);
    for (const auto& entry : dirIt) {
        if (!entry.is_regular_file()) {
            continue;
        }
            
        std::string fullPath = entry.path().string();
        std::ranges::replace(fullPath, '\\', '/');

        std::string relative = std::filesystem::relative(entry.path(), _rootPath).string();
        std::ranges::replace(relative, '\\', '/');

        _pathIndex[relative] = fullPath;
        ++fileCount;
    }
    registerHandlers();
    initPrimitiveMeshes();

#if DZEMIKK_DEV_TOOLS
    auto timer1 = std::chrono::high_resolution_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timer1 - timer0).count();
    spdlog::info("[AssetManager] Initialization finished in {} ms", milliseconds);
#endif
}

std::optional<std::filesystem::path> dzemikk::AssetManager::findResRoot() {
    namespace fs = std::filesystem;

    fs::path start = fs::current_path();

    for (const auto& entry : fs::recursive_directory_iterator(start)) {
        if (entry.is_directory() && entry.path().filename() == "res") {
            return fs::absolute(entry.path());
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

void* dzemikk::AssetManager::loadInternal(const std::string& path, std::type_index type) {
    auto it = _handlers.find(type);
    if (it == _handlers.end()) {
        return nullptr;
    }

    std::string pathR = resolvePath(path); 

    return it->second->load(pathR);
}

std::string dzemikk::AssetManager::resolvePath(const std::string& path) {
    auto it = _pathIndex.find(path);
    if (it != _pathIndex.end()) {
        return it->second;
    }

    return _rootPath + "/" + path;
}

void dzemikk::AssetManager::unload(const std::string& path) {
    auto it = _assets.find(path);
    if (it == _assets.end()) {
        return;
    }

    AssetEntry& entry = it->second;

    auto handlerIt = _handlers.find(entry.type);
    if (handlerIt != _handlers.end()) {
        handlerIt->second->unload(entry.data);
    }

    _assets.erase(it);
}

void dzemikk::AssetManager::initPrimitiveMeshes() {
    _builtinMeshes[PrimitiveMesh::Cube] = PrimitiveFactory::createCube();
    _builtinMeshes[PrimitiveMesh::Quad] = PrimitiveFactory::createQuad();
    _builtinMeshes[PrimitiveMesh::Sphere] = PrimitiveFactory::createSphere();
    _builtinMeshes[PrimitiveMesh::Capsule] = PrimitiveFactory::createCapsule();
}

dzemikk::Mesh* dzemikk::AssetManager::getPrimitive(PrimitiveMesh type) {
    auto it = _builtinMeshes.find(type);
    if (it != _builtinMeshes.end())
        return it->second.get();

    return nullptr;
}

void dzemikk::AssetManager::setFMODSystem(FMOD::System* system) {
    this->_system = system;

    auto handlerIt = _handlers.find(typeid(Sound));
    auto* handler = dynamic_cast<SoundHandler*>(handlerIt->second.get());
    handler->system = system;
}

FMOD::System* dzemikk::AssetManager::getFMODSystem() {
    return _system;
}

void dzemikk::AssetManager::registerHandlers() {
    _handlers[typeid(Mesh)] = std::make_unique<MeshHandler>();
    _handlers[typeid(Shader)] = std::make_unique<ShaderHandler>();
    _handlers[typeid(Texture)] = std::make_unique<TextureHandler>();
    _handlers[typeid(Skybox)] = std::make_unique<SkyboxHandler>();
    _handlers[typeid(Font)] = std::make_unique<FontHandler>();
    _handlers[typeid(Sound)] = std::make_unique<SoundHandler>();
}

void dzemikk::AssetManager::reloadInternal(const std::string& path, void* data, std::type_index type) {
    auto handlerIt = _handlers.find(type);
    if (handlerIt == _handlers.end())
        return;

    handlerIt->second->reload(data, resolvePath(path));
}