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
}

void dzemikk::Engine::update() {
    while (!mainWindow->shouldClose()) {
        mainWindow->clear(0.1f, 0.15f, 0.2f, 1.0f);
        mainWindow->swapBuffers();
        mainWindow->pollEvents();
    }
}

dzemikk::Engine::~Engine() = default;

void dzemikk::Engine::init() {
    spdlog::info("Init");
    mainWindow = std::make_shared<Window>(800, 600, "DZeMIKK");
}