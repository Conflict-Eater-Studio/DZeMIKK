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
    FMOD_RESULT result;
    FMOD::System *system = NULL;

    result = FMOD::System_Create(&system);      // Create the main system object.
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));

    }

    result = system->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }

    unsigned int version = 0;
    result = system->getVersion(&version);

    unsigned int major = (version >> 16) & 0xFFFF;
    unsigned int minor = (version >> 8) & 0xFF;
    unsigned int patch = version & 0xFF;

    spdlog::info("FMOD Version: {}.{}.{}", major, minor, patch);
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
        Time::update();
#if DZEMIKK_DEV_TOOLS
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Renderer");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", Time::deltaTime, 1.0f/Time::deltaTime);
        ImGui::Separator();
        
        const auto& stats = _renderer->getStats();
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

        mainWindow->clear(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#else
        mainWindow->clear(0.1F, 0.15F, 0.2F, 1.0F);
#endif
        _renderer->render();
        mainWindow->swapBuffers();
        mainWindow->pollEvents();
    }

    _renderer->UnInitialize();

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
    FT_Library ft = nullptr;
    if (FT_Init_FreeType(&ft) == 0) {
        FT_Int major = 0;
        FT_Int minor = 0;
        FT_Int patch = 0;
        FT_Library_Version(ft, &major, &minor, &patch);
        spdlog::info("FreeType version: {}.{}.{}", major, minor, patch);
        FT_Done_FreeType(ft);
    } else {
        spdlog::warn("Failed to initialize FreeType (version unavailable)");
    }
#endif

    mainWindow = std::make_shared<Window>(800, 600, "DZeMIKK");

    _renderer = std::make_shared<Renderer>();
    _renderer->Initialize();
}