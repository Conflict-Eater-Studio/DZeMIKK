#include "assetManager/shaderHandler.h"

#include "renderer/shader.h"

#include <iostream>
#include <fstream>
#include <iterator>

const std::string dzemikk::ShaderHandler::VERT = ".vert";
const std::string dzemikk::ShaderHandler::FRAG = ".frag";

dzemikk::ShaderHandler::Result
dzemikk::ShaderHandler::load(const std::string& path,
                             LoadExecutionMode loadExecutionMode) {
    auto shader = loadShaderFromFile(path, loadExecutionMode);

    if (!shader) {
        std::cerr << "Failed to load shader: " << path << "\n";
        return {nullptr, AssetError::LoadFailed};
    }

    return {shader, AssetError::None};
}

std::shared_ptr<dzemikk::Shader>
dzemikk::ShaderHandler::loadShaderFromFile(const std::string& path,
                                           LoadExecutionMode loadExecutionMode) {
    auto [vertPath, fragPath] = buildShaderPaths(path);

    std::ifstream vFile(vertPath);
    std::ifstream fFile(fragPath);

    if (!vFile.is_open() || !fFile.is_open()) {
        std::cerr << "Failed to open shader files:\n" << vertPath << "\n" << fragPath << "\n";
        return nullptr;
    }

    std::string vertSrc((std::istreambuf_iterator<char>(vFile)), std::istreambuf_iterator<char>());

    std::string fragSrc((std::istreambuf_iterator<char>(fFile)), std::istreambuf_iterator<char>());

    auto shader = std::make_shared<Shader>(vertSrc.c_str(), fragSrc.c_str());
    if (loadExecutionMode == LoadExecutionMode::Sync) {
        shader->uploadToGPU();
    }

    return shader;
}

bool dzemikk::ShaderHandler::reload(Handle& asset, const std::string& path) {
    if (!asset)
        return false;

    return reloadShader(path, *asset.get());
}

bool dzemikk::ShaderHandler::reloadShader(const std::string& path, Shader& shader) {
    auto [vertPath, fragPath] = buildShaderPaths(path);

    std::ifstream vFile(vertPath);
    std::ifstream fFile(fragPath);

    if (!vFile.is_open() || !fFile.is_open()) {
        std::cerr << "Failed to reload shader:\n" << vertPath << "\n" << fragPath << "\n";
        return false;
    }

    std::string vertSrc((std::istreambuf_iterator<char>(vFile)), std::istreambuf_iterator<char>());

    std::string fragSrc((std::istreambuf_iterator<char>(fFile)), std::istreambuf_iterator<char>());

    shader.recompile(vertSrc.c_str(), fragSrc.c_str());
    return true;
}

void dzemikk::ShaderHandler::unload(Handle& asset) {
    asset = Handle{};
}

std::pair<std::string, std::string>
dzemikk::ShaderHandler::buildShaderPaths(const std::string& path) {
    return {path + VERT, path + FRAG};
}

