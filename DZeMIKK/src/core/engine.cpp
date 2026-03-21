#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <assimp/version.h>
#include <glm/glm/detail/setup.hpp>

#include "core/engine.h"

dzemikk::Engine::Engine() {
    init();
    spdlog::info("DZeMIKK 1.0.0");
    spdlog::info("GLM version: {}.{}.{}", GLM_VERSION_MAJOR, GLM_VERSION_MINOR, GLM_VERSION_PATCH);
    spdlog::info("Assimp version: {}.{}.{}",
        aiGetVersionMajor(),
        aiGetVersionMinor(),
        aiGetVersionRevision());

    spdlog::info("spdlog version: {}.{}.{}",
        SPDLOG_VER_MAJOR,
        SPDLOG_VER_MINOR,
        SPDLOG_VER_PATCH);


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void dzemikk::Engine::update() {
    spdlog::info("Update");
}

dzemikk::Engine::~Engine() = default;

void dzemikk::Engine::init() {
    spdlog::info("Init");
    mainWindow = std::make_shared<Window>(600, 800, "DZeMIKK");
}
