#include "ecs/componentRegistry.h"
#include "ecs/components/ui/iUIInteractable.h"
#if DZEMIKK_DEV_TOOLS
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <ft2build.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include FT_FREETYPE_H
#endif

#include "animation/animationmodule.h"
#include "assetManager/assetmanager.h"
#include "audio/audioManager.h"
#include "audio/sound.h"
#include "collisions/collisions.h"
#include "core/engine.h"
#include "core/profiler.h"
#include "core/time.h"
#include "core/window.h"
#include "core/windowContext.h"
#include "ecs/components/antiAliasingEffect.h"
#include "ecs/components/camera.h"
#include "ecs/components/colorGradingEffect.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"
#include "ecs/scenemanager.h"
#include "input/input.h"
#include "renderer/Model.h"
#include "renderer/font.h"
#include "renderer/renderer.h"
#include "renderer/texture.h"
#include "spriteAnimation/spriteAnimationModule.h"

#include <GLFW/glfw3.h>
#include <iostream>

namespace dzemikk {

Engine::Engine(EngineMode mode) : _mode(mode) {
    init();
}

Engine::~Engine() {
    shutdown();
}

void Engine::init() {
    _mainWindow = std::make_unique<Window>(1920, 1080, "DZeMIKK", _mode);
    _assetManager = std::make_unique<AssetManager>();
    _renderer = std::make_unique<Renderer>(_mode, this);
    _sceneManager = std::make_unique<SceneManager>();
    _time = std::make_unique<Time>();
    _animationModule = std::make_unique<AnimationModule>();
    _input = std::make_unique<Input>();
    _collisions = std::make_unique<Collisions>();
    _audioManager = std::make_unique<AudioManager>();
    _spriteAnimationModule = std::make_unique<SpriteAnimationModule>();

    _mainWindow->initialize();
    _assetManager->initialize();
    _renderer->initialize();
    _sceneManager->initialize();
    _time->initialize();
    _animationModule->initialize();

    _input->setInputWindow(_mainWindow->nativeHandle());
    _mainWindow->setEventCallback([this](Event& e) { this->OnEvent(e); });
    _input->initialize();
    _collisions->initialize();
    _audioManager->initialize();
    _spriteAnimationModule->initialize();

    _assetManager->setFMODSystem(_audioManager->getSystem());

    _input->OnMouseScrolled.addListener(
        [&](dzemikk::MouseScrolledEvent& e) { _scrollDelta = e.GetYOffset(); });

#if DZEMIKK_DEV_TOOLS
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(_mainWindow->nativeHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    spdlog::info("DZeMIKK version: {}.{}.{}", DZeMIKK_VERSION_MAJOR, DZeMIKK_VERSION_MINOR,
                 DZeMIKK_VERSION_REVISION);
#endif
}

void Engine::shutdown() {
    if (!_mainWindow)
        return;

    _spriteAnimationModule->uninitialize();
    _input->uninitialize();
    _animationModule->uninitialize();
    _time->uninitialize();
    _sceneManager->uninitialize();
#if DZEMIKK_DEV_TOOLS
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#endif

    _audioManager->uninitialize();
    _mainWindow->uninitialize();
    _assetManager->uninitialize();
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
        _audioManager->update(_time->getDeltaTime());

        if (_input) {
            _input->Update();
        }

        float deltaTime = _time->getDeltaTime();
        Profiler::Get().BeginFrame(deltaTime);

        _accumulator += deltaTime;

        {
            DZ_PROFILE_CPU("Game Logic & Update");
            _sceneManager->update(deltaTime);
            _animationModule->update(deltaTime);
            _spriteAnimationModule->update(deltaTime);

            if (_accumulator >= fixedDeltaTime) {
                _sceneManager->fixedUpdate(fixedDeltaTime);
                _accumulator -= fixedDeltaTime;
            }
        }

#if DZEMIKK_DEV_TOOLS
        // ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        _mainWindow->clear(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

#else
        _mainWindow->clear(0.1F, 0.15F, 0.2F, 1.0F);
#endif
        if (_mode == EngineMode::Game) {
            //updateCameraWASD(.3f);
            //updateCameraArrows(0.3f);
            updateMouseUI(deltaTime);
        }

        _collisions->update(this, deltaTime);

        if (m_UserUpdateCallback) {
            m_UserUpdateCallback();
        }

        {
            DZ_PROFILE_CPU("Renderer (Total CPU)");
            _renderer->render();
        }

        _assetManager->update();

#if DZEMIKK_DEV_TOOLS
        if (_mode == EngineMode::Game) {
            Profiler::Get().DrawImGui();

            /*
            std::vector<ColorGradingEffect*> gradingEffects;
            ComponentRegistry::get().getEnabledComponents<ColorGradingEffect>(gradingEffects);
            if (!gradingEffects.empty()) {
                ImGui::Begin("Color Grading Settings");
                for (size_t i = 0; i < gradingEffects.size(); ++i) {
                    auto* effect = gradingEffects[i];
                    std::string label =
                        "Effect " + std::to_string(i) + " (" + effect->getOwner()->getName() + ")";
                    if (ImGui::TreeNode(label.c_str())) {
                        bool enabled = effect->isEnabled();
                        if (ImGui::Checkbox("Enabled", &enabled)) {
                            effect->setEnabled(enabled);
                        }

            std::vector<AntiAliasingEffect*> aaEffects;
            ComponentRegistry::get().getEnabledComponents<AntiAliasingEffect>(aaEffects);

            if (!gradingEffects.empty() || !aaEffects.empty()) {
                ImGui::Begin("Post-Processing Settings");

                if (!gradingEffects.empty()) {
                    if (ImGui::CollapsingHeader("Color Grading", ImGuiTreeNodeFlags_DefaultOpen)) {
                        for (size_t i = 0; i < gradingEffects.size(); ++i) {
                            auto* effect = gradingEffects[i];
                            std::string label = "Color Grading " + std::to_string(i) + " (" +
            effect->getOwner()->getName() + ")"; if (ImGui::TreeNode(label.c_str())) { bool enabled
            = effect->isEnabled(); if (ImGui::Checkbox("Enabled", &enabled)) {
                                    effect->setEnabled(enabled);
                                }

                                float exp = effect->getExposure();
                                if (ImGui::SliderFloat("Exposure", &exp, -4.0f, 4.0f)) {
                                    effect->setExposure(exp);
                                }

                                float contrast = effect->getContrast();
                                if (ImGui::SliderFloat("Contrast", &contrast, 0.0f, 3.0f)) {
                                    effect->setContrast(contrast);
                                }

                                float saturation = effect->getSaturation();
                                if (ImGui::SliderFloat("Saturation", &saturation, 0.0f, 3.0f)) {
                                    effect->setSaturation(saturation);
                                }

                                float temp = effect->getTemperature();
                                if (ImGui::SliderFloat("Temperature", &temp, -2.0f, 2.0f)) {
                                    effect->setTemperature(temp);
                                }

                                float tint = effect->getTint();
                                if (ImGui::SliderFloat("Tint", &tint, -2.0f, 2.0f)) {
                                    effect->setTint(tint);
                                }

                                glm::vec3 filter = effect->getColorFilter();
                                if (ImGui::ColorEdit3("Color Filter", &filter.x)) {
                                    effect->setColorFilter(filter);
                                }

                                ImGui::TreePop();
                            }
                        }
                    }
                }

                if (!aaEffects.empty()) {
                    if (ImGui::CollapsingHeader("Anti-Aliasing (FXAA)",
            ImGuiTreeNodeFlags_DefaultOpen)) { for (size_t i = 0; i < aaEffects.size(); ++i) { auto*
            effect = aaEffects[i]; std::string label = "FXAA " + std::to_string(i) + " (" +
            effect->getOwner()->getName() + ")"; if (ImGui::TreeNode(label.c_str())) { bool enabled
            = effect->isEnabled(); if (ImGui::Checkbox("Enabled##FXAA", &enabled)) {
                                    effect->setEnabled(enabled);
                                }

                                float spanMax = effect->getSpanMax();
                                if (ImGui::SliderFloat("Span Max", &spanMax, 1.0f, 16.0f)) {
                                    effect->setSpanMax(spanMax);
                                }

                                float reduceMul = effect->getReduceMul();
                                if (ImGui::SliderFloat("Reduce Mul", &reduceMul, 0.0f, 0.5f)) {
                                    effect->setReduceMul(reduceMul);
                                }

                                float reduceMin = effect->getReduceMin();
                                if (ImGui::SliderFloat("Reduce Min", &reduceMin, 0.0f, 0.1f,
            "%.5f")) { effect->setReduceMin(reduceMin);
                                }

                                ImGui::TreePop();
                            }
                        }
                    }
                }

                ImGui::End();

            }
            */
        }
        glDisable(GL_DEPTH_TEST);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
        _mainWindow->swapBuffers();
        _mainWindow->pollEvents();
    }
}

void Engine::exit() {
    glfwSetWindowShouldClose(_mainWindow->nativeHandle(), GLFW_TRUE);
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

AnimationModule* Engine::getAnimationModule() const {
    return _animationModule.get();
}

AssetManager* Engine::getAssetManager() const {
    return _assetManager.get();
}

Input* Engine::getInput() const {
    return _input.get();
}

Collisions* Engine::getCollisions() const {
    return _collisions.get();
}

AudioManager* Engine::getAudioManager() const {
    return _audioManager.get();
}

void Engine::updateCameraWASD(float speed) {
    auto* transform = _renderer->getCameraSystem().getActiveSceneCamera()->getOwner()->transform();

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
        transform->translate(move * speed * 0.16f);
    }
}

void Engine::updateCameraArrows(float speed) {
    auto* camera = _renderer->getCameraSystem().getActiveSceneCamera();
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
        getAssetManager()->reload<dzemikk::Model>("models/Flair(1).fbx");
    }
    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_Y) == GLFW_PRESS) {
        getAssetManager()->reload<dzemikk::Shader>("shaders/PBRFresnelGlow");
    }
    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_U) == GLFW_PRESS) {
        getAssetManager()->reload<dzemikk::Skybox>("textures/Daylight Box_Pieces");
    }
    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_O) == GLFW_PRESS) {
        getAssetManager()->reload<dzemikk::Texture>("textures/tex3.png");
    }

    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_J) == GLFW_PRESS) {
        getAssetManager()->get<dzemikk::Model>("models/Body Block.fbx");
    }
    if (glfwGetKey(_mainWindow->nativeHandle(), GLFW_KEY_K) == GLFW_PRESS) {
        getAssetManager()->unload("models/Body Block.fbx");
    }
}

void Engine::updateMouseUI(float deltaTime) {
    (void)deltaTime;

    auto* camera = _renderer->getCameraSystem().getActiveUICamera();
    if (!camera) {
        return;
    }

    double mouseX, mouseY;
    glfwGetCursorPos(_mainWindow->nativeHandle(), &mouseX, &mouseY);

    int width = 0;
    int height = 0;
    glfwGetWindowSize(_mainWindow->nativeHandle(), &width, &height);
    WindowContext::get().setWindowSize({width, height});

    const glm::vec2 pointerPos(static_cast<float>(mouseX),
                               static_cast<float>(height) - static_cast<float>(mouseY));

    const bool isLeftDown =
        glfwGetMouseButton(_mainWindow->nativeHandle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    const bool pressedThisFrame = isLeftDown && !_wasLeftMouseDown;
    const bool releasedThisFrame = !isLeftDown && _wasLeftMouseDown;

    std::vector<IUIInteractable*> uiElements;
    ComponentRegistry::get().getEnabledComponents<IUIInteractable>(uiElements);
    for (auto* element : uiElements) {
        element->processPointer(pointerPos, isLeftDown, pressedThisFrame, releasedThisFrame,
                                _scrollDelta);
    }

    _wasLeftMouseDown = isLeftDown;
    _scrollDelta = 0.0;
}

void Engine::OnEvent(Event& e) {
    if (_input) {
        _input->OnEvent(e);
    }
}

} // namespace dzemikk
