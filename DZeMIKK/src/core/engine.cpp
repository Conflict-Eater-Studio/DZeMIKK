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
    Engine::~Engine() = default;
    void Engine::init() {
        _mainWindow = std::make_shared<Window>(800, 600, "DZeMIKK");
        _renderer = std::make_shared<Renderer>();
        _sceneManager = std::make_shared<SceneManager>();
        _time = std::make_shared<Time>();

        _modules.push_back(_renderer);
        _modules.push_back(_sceneManager);
        for (const auto& element : _modules) {
            element->Initialize();
        }
    #if DZEMIKK_DEV_TOOLS
        spdlog::info("DZeMIKK version: {}.{}.{}", DZeMIKK_VERSION_MAJOR, DZeMIKK_VERSION_MINOR, DZeMIKK_VERSION_REVISION);
    #endif
    }

    void Engine::start() const {

    }

    void Engine::update() {
    #if DZEMIKK_DEV_TOOLS
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(_mainWindow->nativeHandle(), true);
        ImGui_ImplOpenGL3_Init("#version 330");

        ImVec4 clear_color = ImVec4(0.10F, 0.15F, 0.20F, 1.00F);
    #endif
        while (!_mainWindow->shouldClose()) {
            _time->update();
            float dt = _time->getDeltaTime();
            _accumulator += dt;

            //_sceneManager->update(dt);

            float fdt = _time->getFixedDeltaTime();
            if (_accumulator >= fdt) {
                _accumulator -= fdt;
                //_sceneManager->fixedUpdate(fdt);
            }
    #if DZEMIKK_DEV_TOOLS
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Renderer");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", _time->getDeltaTime(), 1.0f/_time->getDeltaTime());
            ImGui::Text("Background");
            ImGui::ColorEdit4("Clear Color", reinterpret_cast<float*>(&clear_color));
            ImGui::End();

            _mainWindow->clear(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    #else
            _mainWindow->clear(0.1F, 0.15F, 0.2F, 1.0F);
    #endif
            _renderer->render();
            _mainWindow->swapBuffers();
            _mainWindow->pollEvents();
        }

        _renderer->UnInitialize();

    #if DZEMIKK_DEV_TOOLS
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    #endif
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