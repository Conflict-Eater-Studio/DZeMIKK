#include "assetManager/skyboxHandler.h"

#include "renderer/skybox.h"
#include <iostream>

void* dzemikk::SkyboxHandler::load(const std::string& path) {
    return loadSkyboxFromFile(path).release();
}

std::unique_ptr<dzemikk::Skybox> dzemikk::SkyboxHandler::loadSkyboxFromFile(const std::string& path) {
    std::vector<std::string> faces = buildFaces(path);

    auto skybox = std::make_unique<dzemikk::Skybox>();

    try {
        skybox->loadCubemap(faces);
    } catch (const std::exception& e) {
        std::cerr << "[AssetManager] Skybox load failed: " << e.what() << "\n";
        return nullptr;
    }

    return skybox;
}

void dzemikk::SkyboxHandler::reload(void* asset, const std::string& path) {
    reloadSkybox(path, static_cast<Skybox*>(asset));
}

void dzemikk::SkyboxHandler::reloadSkybox(const std::string& path, dzemikk::Skybox* skybox) {
    std::vector<std::string> faces = buildFaces(path);

    try {
        skybox->loadCubemap(faces);
    } catch (const std::exception& e) {
        std::cerr << "[AssetManager] Skybox reload failed: " << e.what() << "\n";
    }
}

void dzemikk::SkyboxHandler::unload(void* asset) {
    delete static_cast<Skybox*>(asset);
}

std::vector<std::string> dzemikk::SkyboxHandler::buildFaces(const std::string& path) {
    using namespace dzemikk::skyboxConst;

    return {path + RIGHT, path + LEFT, path + TOP, path + BOTTOM, path + FRONT, path + BACK};
}


