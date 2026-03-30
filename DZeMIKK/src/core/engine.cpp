#include <assimp/version.h>
#include <glm/detail/setup.hpp>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include "core/engine.h"
#include "core/time.h"

#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace dzemikk {

Engine::Engine() {
    init();
}

Engine::~Engine() {
    shutdown();
}

void Engine::init() {
    _mainWindow = std::make_shared<Window>(800, 600, "DZeMIKK");
    _renderer = std::make_shared<Renderer>();
    _sceneManager = std::make_shared<SceneManager>();
    _time = std::make_shared<Time>();
    _animationSystem = std::make_shared<AnimationModule>();

    _modules.push_back(_mainWindow);
    _modules.push_back(_renderer);
    _modules.push_back(_sceneManager);
    _modules.push_back(_time);
    _modules.push_back(_animationSystem);

    for (const auto& module : _modules) {
        module->Initialize();
    }

#if DZEMIKK_DEV_TOOLS
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(_mainWindow->nativeHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    spdlog::info("DZeMIKK version: {}.{}.{}",
        DZeMIKK_VERSION_MAJOR,
        DZeMIKK_VERSION_MINOR,
        DZeMIKK_VERSION_REVISION);
#endif
}

void Engine::shutdown() {
#if DZEMIKK_DEV_TOOLS
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#endif

    for (const auto& module : _modules) {
        module->UnInitialize();
    }
}

void Engine::start() {

#if DZEMIKK_DEV_TOOLS
    ImVec4 clear_color = ImVec4(0.10F, 0.15F, 0.20F, 1.00F);
#endif

    float fdt = _time->getFixedDeltaTime();
    while (!_mainWindow->shouldClose()) {
        _time->update();
        float dt = _time->getDeltaTime();
        _accumulator += dt;

        _sceneManager->update(dt);
        _animationSystem->update(dt);

        if (_accumulator >= fdt) {
            _sceneManager->fixedUpdate(fdt);
            _accumulator -= fdt;
        }

#if DZEMIKK_DEV_TOOLS
        // ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Renderer");

        float dt_ms = dt * 1000.0f;
        ImGui::Text("Application %.3f ms/frame (%.1f FPS)",
            dt_ms,
            1.0f / dt);

        ImGui::ColorEdit4("Clear Color",
            reinterpret_cast<float*>(&clear_color));

        ImGui::End();

        _mainWindow->clear(
            clear_color.x,
            clear_color.y,
            clear_color.z,
            clear_color.w
        );

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#else
        _mainWindow->clear(0.1F, 0.15F, 0.2F, 1.0F);
#endif

        _renderer->render();

        _mainWindow->swapBuffers();
        _mainWindow->pollEvents();
    }
}

std::shared_ptr<Renderer> Engine::getRenderer() {
    return _renderer;
}

std::shared_ptr<Window> Engine::getWindow() {
    return _mainWindow;
}

std::shared_ptr<SceneManager> Engine::getSceneManager() {
    return _sceneManager;
}

std::shared_ptr<Time> Engine::getTime() {
    return _time;
}
std::shared_ptr<AnimationModule> Engine::getAnimationSystem() {
    return _animationSystem;
}

template <std::derived_from<IEngineModule> T>
std::shared_ptr<T> Engine::getModule() const {
    for (const auto& module : _modules) {
        if (auto casted = std::dynamic_pointer_cast<T>(module)) {
            return casted;
        }
    }
    return nullptr;
}

}