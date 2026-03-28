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

void dzemikk::Engine::update() {
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
        ImGui::Text("Background");
        ImGui::ColorEdit4("Clear Color", reinterpret_cast<float*>(&clear_color));
        ImGui::End();

        mainWindow->clear(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#else
        mainWindow->clear(0.1F, 0.15F, 0.2F, 1.0F);
#endif
        updateCameraWASD(.1f);
        updateCameraArrows(.1f); 
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

    mainWindow = std::make_shared<Window>(1920, 1080, "DZeMIKK");

    _renderer = std::make_shared<Renderer>();
    _renderer->Initialize();
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

    glm::vec3 euler = transform->getEulerAngles();

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_LEFT) == GLFW_PRESS)
        euler.y += speed;

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_RIGHT) == GLFW_PRESS)
        euler.y -= speed;

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_UP) == GLFW_PRESS)
        euler.x += speed;

    if (glfwGetKey(mainWindow->nativeHandle(), GLFW_KEY_DOWN) == GLFW_PRESS)
        euler.x -= speed;

    transform->setEulerAngles(euler);
}