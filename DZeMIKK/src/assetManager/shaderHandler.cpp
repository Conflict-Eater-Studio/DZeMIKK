#include "assetManager/shaderHandler.h"

#include "renderer/shader.h"

#include <iostream>
#include <fstream>
#include <iterator>

const std::string dzemikk::ShaderHandler::VERT = ".vert";
const std::string dzemikk::ShaderHandler::FRAG = ".frag";

void* dzemikk::ShaderHandler::load(const std::string& path) {
    return loadShaderFromFile(path);
}

dzemikk::Shader* dzemikk::ShaderHandler::loadShaderFromFile(const std::string& path) {
    auto [vertPath, fragPath] = buildShaderPaths(path);

    std::ifstream vFile(vertPath);
    std::ifstream fFile(fragPath);

    if (!vFile.is_open() || !fFile.is_open()) {
        std::cerr << "Failed to open shader:\n" << vertPath << "\n" << fragPath << "\n";
        return nullptr;
    }

    std::string vertSrc((std::istreambuf_iterator<char>(vFile)), std::istreambuf_iterator<char>());

    std::string fragSrc((std::istreambuf_iterator<char>(fFile)), std::istreambuf_iterator<char>());

    return new dzemikk::Shader(vertSrc.c_str(), fragSrc.c_str());
}

void dzemikk::ShaderHandler::reload(void* asset, const std::string& path) {
    reloadShader(path, static_cast<Shader*>(asset));
}

void dzemikk::ShaderHandler::reloadShader(const std::string& path, dzemikk::Shader* shader) {
    auto [vertPath, fragPath] = buildShaderPaths(path);

    std::ifstream vFile(vertPath);
    std::ifstream fFile(fragPath);

    if (!vFile.is_open() || !fFile.is_open()) {
        std::cerr << "Failed to reload shader\n";
        return;
    }

    std::string vertSrc((std::istreambuf_iterator<char>(vFile)), {});
    std::string fragSrc((std::istreambuf_iterator<char>(fFile)), {});

    shader->recompile(vertSrc.c_str(), fragSrc.c_str());
}

void dzemikk::ShaderHandler::unload(void* asset) {
    delete static_cast<Shader*>(asset);
}

std::pair<std::string, std::string>
dzemikk::ShaderHandler::buildShaderPaths(const std::string& path) {
    return {path + VERT, path + FRAG};
}

