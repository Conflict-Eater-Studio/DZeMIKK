#include "assetManager/skyboxHandler.h"

#include "assetManager/assetError.h"
#include "renderer/skybox.h"

#include <iostream>

dzemikk::SkyboxHandler::Result dzemikk::SkyboxHandler::load(const std::string& path) {
    auto skybox = loadSkyboxFromFile(path);

    if (!skybox) {
        std::cerr << "[AssetManager] Skybox load failed: " << path << "\n";
        return {Handle(), nullptr, AssetError::LoadFailed};
    }

    return {Handle(skybox.get()), skybox, AssetError::None};
}

std::shared_ptr<dzemikk::Skybox> dzemikk::SkyboxHandler::loadSkyboxFromFile(const std::string& path) {
    std::vector<std::string> faces = buildFaces(path);

    auto skybox = std::make_shared<Skybox>();

    try {
        skybox->loadCubemap(faces);
    } catch (const std::exception& e) {
        std::cerr << "[AssetManager] Skybox load failed: " << e.what() << "\n";
        return nullptr;
    }

    return skybox;
}

bool dzemikk::SkyboxHandler::reloadSkybox(const std::string& path, Skybox& skybox) {
    std::vector<std::string> faces = buildFaces(path);

    try {
        skybox.loadCubemap(faces);
    } catch (const std::exception& e) {
        std::cerr << "[AssetManager] Skybox reload failed: " << e.what() << "\n";
        return false;
    }

    return true;
}

bool dzemikk::SkyboxHandler::reload(Handle& asset, const std::string& path) {
    if (!asset.valid())
        return false;

    return reloadSkybox(path, *asset);
}

void dzemikk::SkyboxHandler::unload(Handle& asset) {
    asset = Handle{};
}

std::vector<std::string> dzemikk::SkyboxHandler::buildFaces(const std::string& path) {
    using namespace dzemikk::skyboxConst;

    return {path + RIGHT, path + LEFT, path + TOP, path + BOTTOM, path + FRONT, path + BACK};
}