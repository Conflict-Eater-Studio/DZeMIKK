#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

#include "assetManager/assetmanager.h"
#include "assetManager/fontHandler.h"
#include "assetManager/meshHandler.h"
#include "assetManager/primitiveFactory.h"
#include "assetManager/shaderHandler.h"
#include "assetManager/skyboxHandler.h"
#include "assetManager/soundHandler.h"
#include "assetManager/textureHandler.h"
#include "audio/sound.h"
#include "renderer/font.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include "renderer/skybox.h"
#include "renderer/texture.h"

#include <algorithm>
#include <assimp/version.h>
#include <corecrt_math_defines.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stb/stb_image.h>

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

    for (const auto& entry : std::filesystem::recursive_directory_iterator(_rootPath)) {
        if (!entry.is_regular_file())
            continue;

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
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timer1 - timer0).count();
    spdlog::info("[AssetManager] Initialization finished in {} ms", ms);
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
    _assets.clear();
    _builtinMeshes.clear();
    _handlers.clear();
    _pathIndex.clear();
}

std::string dzemikk::AssetManager::resolvePath(const std::string& path) {
    auto it = _pathIndex.find(path);
    if (it != _pathIndex.end())
        return it->second;

    return (std::filesystem::path(_rootPath) / path).generic_string();
}

void dzemikk::AssetManager::unload(const std::string& path) {
    auto it = _assets.find(path);
    if (it == _assets.end())
        return;

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
    _system = system;

    auto it = _handlers.find(typeid(Sound));
    if (it != _handlers.end()) {
        auto* handler = static_cast<SoundHandler*>(it->second.get());
        handler->system = system;
    }
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