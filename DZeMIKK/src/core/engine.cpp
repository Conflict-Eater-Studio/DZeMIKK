#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include "animation/animationmodule.h"
#include "core/engine.h"
#include "core/time.h"
#include "core/window.h"
#include "ecs/components/camera.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"
#include "ecs/scenemanager.h"
#include "renderer/renderer.h"
#include "assetManager/assetmanager.h"

#include "core/profiler.h"

#include <GLFW/glfw3.h>
#include <iostream>
namespace dzemikk {

Engine::Engine() {
    init();
}

Engine::~Engine() {
    shutdown();
}

void Engine::init() {
    _mainWindow = std::make_shared<Window>(1920, 1080, "DZeMIKK");
    _assetManager = std::make_shared<AssetManager>();
    _renderer = std::make_shared<Renderer>();
    _sceneManager = std::make_shared<SceneManager>();
    _time = std::make_shared<Time>();
    _animationSystem = std::make_shared<AnimationModule>();

    _modules.push_back(_assetManager);
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

    float fixedDeltaTime = _time->getFixedDeltaTime();
    while (!_mainWindow->shouldClose()) {
        _time->update();

        float deltaTime = _time->getDeltaTime();
        _accumulator += deltaTime;

        _sceneManager->update(deltaTime);

        _animationSystem->update(deltaTime);

        if (_accumulator >= fixedDeltaTime) {
            _sceneManager->fixedUpdate(fixedDeltaTime);
            _accumulator -= fixedDeltaTime;
        }

#if DZEMIKK_DEV_TOOLS
        // ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Renderer");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", _time->deltaTime,
                    1.0f / _time->deltaTime);
        ImGui::Separator();
        
        const auto& stats = Profiler::rendererStats;
        ImGui::Text("Render Stats:");
        ImGui::Text("Draw Calls:      %u", stats.drawCalls);
        ImGui::Text("Objects:         %u", stats.renderedObjects);
        ImGui::Text("Triangles:       %u", stats.triangleCount);
        ImGui::Text("Vertices:        %u", stats.vertexCount);
        ImGui::Text("State Changes:   %u", stats.stateChanges);
        
        ImGui::Separator();
        ImGui::Text("Background");
        ImGui::ColorEdit4("Clear Color", reinterpret_cast<float*>(&clear_color));
        ImGui::End();

        _mainWindow->clear(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

#else
        _mainWindow->clear(0.1F, 0.15F, 0.2F, 1.0F);
#endif
        updateCameraWASD(1.f);
        updateCameraArrows(1.1f);
        _renderer->render();
#if DZEMIKK_DEV_TOOLS
        glDisable(GL_DEPTH_TEST);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
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

std::shared_ptr<AssetManager> Engine::getAssetManager() {
    return _assetManager;
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

void Engine::updateCameraWASD(float speed) {
    auto* transform = _renderer->getActiveSceneCamera()->getOwner()->transform();

    glm::vec3 move(0.0f);

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_W) == GLFW_PRESS)
        move += transform->forward();

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_S) == GLFW_PRESS)
        move -= transform->forward();

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_A) == GLFW_PRESS)
        move -= transform->right();

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_D) == GLFW_PRESS)
        move += transform->right();

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_Q) == GLFW_PRESS)
        move -= transform->up();

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_E) == GLFW_PRESS)
        move += transform->up();

    if (glm::length(move) > 0.0f) {
        move = glm::normalize(move);
        transform->translate(move * speed * 0.016f);
    }
}

void Engine::updateCameraArrows(float speed) {
    auto* camera = _renderer->getActiveSceneCamera();
    if (!camera)
        return;
    auto* transform = camera->getOwner()->transform();

    float deltaAngle = speed;

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_LEFT) == GLFW_PRESS)
        transform->rotate(glm::angleAxis(glm::radians(deltaAngle), transform->up()));

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_RIGHT) == GLFW_PRESS)
        transform->rotate(glm::angleAxis(glm::radians(-deltaAngle), transform->up()));

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_UP) == GLFW_PRESS)
        transform->rotate(glm::angleAxis(glm::radians(deltaAngle), transform->right()));

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_DOWN) == GLFW_PRESS)
        transform->rotate(glm::angleAxis(glm::radians(-deltaAngle), transform->right()));

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_Z) == GLFW_PRESS) {
            getAssetManager()->Reload<dzemikk::Mesh>("models/pole.fbx");
        std::cout<<"reload";
    }

}
}