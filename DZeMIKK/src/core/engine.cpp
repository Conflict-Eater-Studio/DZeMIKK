#include <assimp/version.h>
#include <glm/glm/detail/setup.hpp>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#endif

#include "core/engine.h"

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

dzemikk::Engine::Engine() {
    init();
}

void dzemikk::Engine::update() const {
#if DZEMIKK_DEV_TOOLS
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(mainWindow->nativeHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImVec4 clear_color = ImVec4(0.10F, 0.15F, 0.20F, 1.00F);
#endif
    while (!mainWindow->shouldClose()) {
#if DZEMIKK_DEV_TOOLS
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Renderer");
        ImGui::Text("Background");
        ImGui::ColorEdit4("Clear Color", reinterpret_cast<float*>(&clear_color));
        ImGui::End();

        mainWindow->clear(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#else
        mainWindow->clear(0.1F, 0.15F, 0.2F, 1.0F);
#endif
        renderer->DrawCube();
        renderer->DrawRectangle();
        mainWindow->swapBuffers();
        mainWindow->pollEvents();
    }

#if DZEMIKK_DEV_TOOLS
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#endif
}

dzemikk::Engine::~Engine() = default;

void dzemikk::Engine::init() {
#if DZEMIKK_DEV_TOOLS
    spdlog::info("DZeMIKK version: {}.{}.{}", DZeMIKK_VERSION_MAJOR, DZeMIKK_VERSION_MINOR, DZeMIKK_VERSION_REVISION);
    spdlog::info("GLM version: {}.{}.{}", GLM_VERSION_MAJOR, GLM_VERSION_MINOR, GLM_VERSION_PATCH);
    spdlog::info("Assimp version: {}.{}.{}",
                 aiGetVersionMajor(),
                 aiGetVersionMinor(),
                 aiGetVersionRevision());
    spdlog::info("spdlog version: {}.{}.{}",
                 SPDLOG_VER_MAJOR,
                 SPDLOG_VER_MINOR,
                 SPDLOG_VER_PATCH);
#endif

    mainWindow = std::make_shared<Window>(800, 600, "DZeMIKK");
    renderer = std::make_shared<Renderer>();
}