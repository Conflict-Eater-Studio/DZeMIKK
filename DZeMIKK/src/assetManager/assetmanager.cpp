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
#include "assetManager/modelHandler.h"

#include "audio/sound.h"

#include "renderer/font.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include "renderer/skybox.h"
#include "renderer/texture.h"
#include "renderer/model.h"

#include <algorithm>
#include <assimp/version.h>
#include <corecrt_math_defines.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stb/stb_image.h>

void dzemikk::AssetManager::initialize() {
#if DZEMIKK_DEV_TOOLS
    auto timer0 = std::chrono::high_resolution_clock::now();
    spdlog::info("[AssetManager] Initialization started");

    spdlog::info("Assimp version: {}.{}.{}", aiGetVersionMajor(), aiGetVersionMinor(),
                 aiGetVersionRevision());
#endif

    if (!_resources.initialize()) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("[AssetManager] ResourceIndex init failed!");
#endif
        return;
    }

    registerHandlers();

#if DZEMIKK_DEV_TOOLS
    auto timer1 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timer1 - timer0).count();
    spdlog::info("[AssetManager] Initialization finished in {} ms", ms);
#endif

    _threadPool.start();
}

void dzemikk::AssetManager::uninitialize() {
    _database.clear();
    _loaders.clear();
    _resources.clear();
    _primitiveMeshLibrary.clear();
}

void dzemikk::AssetManager::unload(const std::string& path) {
    auto type = _database.getType(path);
    if (type == typeid(void)) {
        return;
    }

    auto asset = _database.getRaw(path);
    if (!asset) {
        return;
    }

    auto* handler = _loaders.getByType(type);
    if (!handler) {
        return;
    }

    handler->unloadUntyped(asset, path);

    _database.remove(path);
}

dzemikk::AssetHandle<dzemikk::Model> dzemikk::AssetManager::getPrimitiveModel(PrimitiveMeshLibrary::PrimitiveMesh type) {
    const std::string key = "primitive/" + std::to_string(static_cast<int>(type));

    if (auto cached = _database.get<Model>(key)) {
#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetManager] Primitive model from cache: {}", key);
#endif
        return AssetHandle<Model>(cached, key);
    }

    auto model = std::make_shared<Model>();
    model->addMesh(_primitiveMeshLibrary.get(type), 0);

    _database.store<Model>(key, model);

    return AssetHandle<Model>(model, key);
}

dzemikk::Mesh* dzemikk::AssetManager::getPrimitive(PrimitiveMeshLibrary::PrimitiveMesh type) {
    return _primitiveMeshLibrary.get(type).get();
}

void dzemikk::AssetManager::setFMODSystem(FMOD::System* system) {
    _system = system;

    auto* base = _loaders.get<Sound>();
    if (base) {
        auto* handler = dynamic_cast<SoundHandler*>(base);
        handler->system = system;
    }
}

FMOD::System* dzemikk::AssetManager::getFMODSystem() const{
    return _system;
}

void dzemikk::AssetManager::processGpuUploads() {
    std::lock_guard lock(_gpuMutex);

    while (!_gpuUploadQueue.empty()) {
        auto gpu = _gpuUploadQueue.front();
        _gpuUploadQueue.pop();

        if (gpu) {
            gpu->uploadToGPU();
        }
    }
}

void dzemikk::AssetManager::registerHandlers() {
    _loaders.registerHandler<Mesh>(std::make_unique<MeshHandler>());
    _loaders.registerHandler<Shader>(std::make_unique<ShaderHandler>());
    _loaders.registerHandler<Texture>(std::make_unique<TextureHandler>());
    _loaders.registerHandler<Skybox>(std::make_unique<SkyboxHandler>());
    _loaders.registerHandler<Font>(std::make_unique<FontHandler>());
    _loaders.registerHandler<Sound>(std::make_unique<SoundHandler>());
    _loaders.registerHandler<Model>(std::make_unique<ModelHandler>());
}

void dzemikk::AssetManager::update() {
    processGpuUploads();
}

dzemikk::AssetHandle<dzemikk::Mesh> dzemikk::AssetManager::getPrimitiveMesh(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh type) {
    const std::string key = "primitive_mesh/" + std::to_string(static_cast<int>(type));

    if (auto cached = _database.get<Mesh>(key)) {
#if DZEMIKK_DEV_TOOLS
        spdlog::info("[AssetManager] Primitive mesh from cache: {}", key);
#endif
        return AssetHandle<Mesh>(cached, key);
    }

    auto mesh = _primitiveMeshLibrary.get(type);

    if (!mesh) {
        return {};
    }

    _database.store<Mesh>(key, mesh);

    return AssetHandle<Mesh>(mesh, key);
}