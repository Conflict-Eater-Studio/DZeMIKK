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

dzemikk::Engine::Engine() {
    init();
}
dzemikk::Engine::~Engine() = default;
void dzemikk::Engine::init() {
    for (const auto& element : _modules) {
        element->Initialize();
    }
#if DZEMIKK_DEV_TOOLS
    spdlog::info("DZeMIKK version: {}.{}.{}", DZeMIKK_VERSION_MAJOR, DZeMIKK_VERSION_MINOR, DZeMIKK_VERSION_REVISION);
#endif
    _mainWindow = std::make_shared<Window>(800, 600, "DZeMIKK");
    _renderer = std::make_shared<Renderer>();
    _renderer->Initialize();
}

void dzemikk::Engine::start() const {

}

void dzemikk::Engine::update() const {
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
        Time::update();
#if DZEMIKK_DEV_TOOLS
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Renderer");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", Time::deltaTime, 1.0f/Time::deltaTime);
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
void dzemikk::Engine::fixedUpdate() const {

}
void dzemikk::Engine::lateUpdate() const {

}
template <std::derived_from<dzemikk::IEngineModule> T>
std::shared_ptr<T> dzemikk::Engine::getModule() const {
    for (const auto& module : _modules) {
        if (auto casted = dynamic_cast<std::shared_ptr<T>>(module.get())) {
            return casted;
        }
    }
    return;
}



