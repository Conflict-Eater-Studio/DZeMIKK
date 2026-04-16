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
#include "renderer/font.h"
#include "renderer/texture.h"
#include "audio/sound.h"
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
    _mainWindow = std::make_unique<Window>(1920, 1080, "DZeMIKK");
    _assetManager = std::make_unique<AssetManager>();
    _renderer = std::make_unique<Renderer>();
    _sceneManager = std::make_unique<SceneManager>();
    _time = std::make_unique<Time>();
    _animationModule = std::make_unique<AnimationModule>();

    _mainWindow->initialize();
    _assetManager->initialize();
    _renderer->initialize();
    _sceneManager->initialize();
    _time->initialize();
    _animationModule->initialize();

    // _modules.push_back(std::move(_assetManager));
    // _modules.push_back(_mainWindow);
    // _modules.push_back(_renderer);
    // _modules.push_back(_sceneManager);
    // _modules.push_back(_time);
    // _modules.push_back(_animationModule);
    //
    // for (const auto& module : _modules) {
    //     module->Initialize();
    // }

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
    _animationModule->uninitialize();
    _time->uninitialize();
    _sceneManager->uninitialize();
    _renderer->uninitialize();
    _assetManager->uninitialize();
    _mainWindow->uninitialize();
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

        _animationModule->update(deltaTime);

        if (_accumulator >= fixedDeltaTime) {
            _sceneManager->fixedUpdate(fixedDeltaTime);
            _accumulator -= fixedDeltaTime;
        }

#if DZEMIKK_DEV_TOOLS
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
Renderer* Engine::getRenderer() const {
    return _renderer.get();
}

Window* Engine::getWindow() const {
    return _mainWindow.get();
}
SceneManager* Engine::getSceneManager() const {
    return _sceneManager.get();

}

Time* Engine::getTime() const {
    return _time.get();
}

AnimationModule* Engine::getAnimationModule() const{
    return _animationModule.get();
}

AssetManager* Engine::getAssetManager() const {
    return _assetManager.get();
}

// template <std::derived_from<IEngineModule> T>
// std::shared_ptr<T> Engine::getModule() const {
//     for (const auto& module : _modules) {
//         if (auto casted = std::dynamic_pointer_cast<T>(module)) {
//             return casted;
//         }
//     }
//     return nullptr;
// }

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

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_R) == GLFW_PRESS) {
        getAssetManager()->reload<dzemikk::Font>("fonts/UncialAntiqua-Regular.ttf");
    }
    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_T) == GLFW_PRESS) {
        getAssetManager()->reload<dzemikk::Mesh>("models/pole.fbx");
    }
    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_Y) == GLFW_PRESS) {
        getAssetManager()->reload<dzemikk::Shader>("shaders/quad");
    }
    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_U) == GLFW_PRESS) {
        getAssetManager()->reload<dzemikk::Skybox>("textures/Daylight Box_Pieces");
    }
    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_I) == GLFW_PRESS) {
        getAssetManager()->reload<dzemikk::Sound>("audio/prime_coToZaHex.wav");
        auto sound = getAssetManager()->get<dzemikk::Sound>("audio/prime_coToZaHex.wav");
        sound.get()->play(getAssetManager()->getFMODSystem());
    }
    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_O) == GLFW_PRESS) {
        getAssetManager()->reload<dzemikk::Texture>("textures/tex3.png");
    }

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_J) == GLFW_PRESS) {
        getAssetManager()->get<dzemikk::Mesh>("models/pole.fbx");
    }
    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_K) == GLFW_PRESS) {
        getAssetManager()->unload("models/pole.fbx");
    }
}
}