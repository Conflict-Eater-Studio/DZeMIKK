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
#include "ecs/gameobject.h"
#include "ecs/components/camera.h" 

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
    mainWindow = std::make_shared<Window>(1920, 1080, "DZeMIKK");
    _renderer = std::make_shared<Renderer>();
    _sceneManager = std::make_shared<SceneManager>();
    _time = std::make_shared<Time>();

    _modules.push_back(_mainWindow);
    _modules.push_back(_renderer);
    _modules.push_back(_sceneManager);
    _modules.push_back(_time);

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

    while (!_mainWindow->shouldClose()) {
        _time->update();
        float dt = _time->getDeltaTime();
        _accumulator += dt;

        _sceneManager->update(dt);

        float fdt = _time->getFixedDeltaTime();
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
        // --- Only for test DELETE THIS
        if (scene)
            scene->update(Time::deltaTime);

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
    return getModule<Renderer>();
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

void dzemikk::Engine::updateCameraWASD(float speed) {
    auto* transform = _renderer->getActiveSceneCamera()->getOwner()->transform();

    glm::vec3 move(0.0f);

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_W) == GLFW_PRESS)
        move += transform->forward();

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_S) == GLFW_PRESS)
        move -= transform->forward();

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_A) == GLFW_PRESS)
        move -= transform->right();

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_D) == GLFW_PRESS)
        move += transform->right();

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_Q) == GLFW_PRESS)
        move -= transform->up(); 

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_E) == GLFW_PRESS)
        move += transform->up();

    if (glm::length(move) > 0.0f) {
        move = glm::normalize(move);
        transform->translate(move * speed * 0.016f);
    }
}

void dzemikk::Engine::updateCameraArrows(float speed) {
    auto* camera = _renderer->getActiveSceneCamera();
    if (!camera)
        return;
    auto* transform = camera->getOwner()->transform();

    float deltaAngle = speed; 

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_LEFT) == GLFW_PRESS)
        transform->rotate(glm::angleAxis(glm::radians(deltaAngle), transform->up()));

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_RIGHT) == GLFW_PRESS)
        transform->rotate(glm::angleAxis(glm::radians(-deltaAngle), transform->up()));

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_UP) == GLFW_PRESS)
        transform->rotate(glm::angleAxis(glm::radians(deltaAngle), transform->right()));

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_DOWN) == GLFW_PRESS)
        transform->rotate(glm::angleAxis(glm::radians(-deltaAngle), transform->right()));
}