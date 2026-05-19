#include "renderer/renderer.h"

#include "ecs/component.h"
#include "ecs/componentRegistry.h"
#include "ecs/components/camera.h"

#include "renderer/renderPasses/skyboxRenderPass.h"
#include "renderer/renderPasses/meshRenderPass.h"
#include "renderer/renderPasses/skinnedRenderPass.h"
#include "renderer/renderPasses/spriteRenderPass.h"
#include "renderer/renderPasses/imageRenderPass.h"
#include "renderer/renderPasses/textRenderPass.h"
#include "renderer/renderPasses/uITextRenderPass.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <map>

#if DZEMIKK_DEV_TOOLS
#include <imgui.h>
#endif

void dzemikk::Renderer::initialize() {
    if (_engineMode == EngineMode::Editor) {
        _sceneFramebuffer = std::make_unique<Framebuffer>(_viewportWidth, _viewportHeight);
    }

    _context = RenderContext(_cameraSystem.getActiveSceneCamera(),
                             _cameraSystem.getActiveUICamera(), glm::mat4(1.0f),
                             glm::mat4(1.0f),
                                 glm::ortho(0.0f, 1920.0f, 0.0f, 1080.0f), _uboMatrices, &_frustum);

    addPass<SkyboxRenderPass>();
    addPass<MeshRenderPass>();
    addPass<SkinnedRenderPass>();
    addPass<SpriteRenderPass>();
    addPass<ImageRenderPass>();
    addPass<TextRenderPass>();
    addPass<UITextRenderPass>();

    glGenBuffers(1, &_uboMatrices);

    glBindBuffer(GL_UNIFORM_BUFFER, _uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, _uboMatrices, 0, 2 * sizeof(glm::mat4));

    glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _context.uboMatrices = _uboMatrices;
}

void dzemikk::Renderer::uninitialize() {
    if (_uboMatrices != 0) {
        glDeleteBuffers(1, &_uboMatrices);
        _uboMatrices = 0;
    }
}

void dzemikk::Renderer::render() {
    _lightSystem.update(_context);

    if (_engineMode == EngineMode::Editor) {
        _sceneFramebuffer->bind();
        glViewport(0, 0, _viewportWidth, _viewportHeight);
    }

    setupFrame();
    _cameraSystem.update(_context);
    _context.sceneCamera = _cameraSystem.getActiveSceneCamera();
    _context.uiCamera = _cameraSystem.getActiveUICamera();

    for (auto& pass : _passes)
        pass->execute(_context);

    if (_engineMode == EngineMode::Editor) {
        _sceneFramebuffer->unbind();
    }

#if DZEMIKK_DEV_TOOLS
    drawDebugUI();
#endif
}

void dzemikk::Renderer::setSkybox(AssetHandle<Skybox> skybox) {
    if (!skybox.get()) {
        return;
    }
    
    auto* pass = getPass<SkyboxRenderPass>();
    if (pass) {
        pass->setSkybox(skybox);
    }
}

const dzemikk::AssetHandle<dzemikk::Skybox> dzemikk::Renderer::getSkybox() const {
    auto pass = getPass<SkyboxRenderPass>();
    if (!pass)
        return {}; 

    return pass->getSkybox();
}

void dzemikk::Renderer::setupFrame() {
    if (_engineMode == EngineMode::Editor) {
        glViewport(0, 0, _viewportWidth, _viewportHeight);    
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

#if DZEMIKK_DEV_TOOLS

void dzemikk::Renderer::drawDebugUI() {

    ImGui::Begin("Renderer Debug");

    // =========================================================
    // LIGHTING
    // =========================================================

    ImGui::SeparatorText("Lighting");

    auto& dirLights = _lightSystem.dirComponents();
    auto& pointLights = _lightSystem.pointComponents();
    auto& spotLights = _lightSystem.spotComponents();

    ImGui::Text("Directional Lights: %d", (int)dirLights.size());
    ImGui::Text("Point Lights: %d", (int)pointLights.size());
    ImGui::Text("Spot Lights: %d", (int)spotLights.size());

    // =========================================================
    // DIRECTIONAL LIGHTS
    // =========================================================

    if (ImGui::CollapsingHeader("Directional Lights", ImGuiTreeNodeFlags_DefaultOpen)) {

        for (size_t i = 0; i < dirLights.size(); ++i) {

            auto* light = dirLights[i];

            ImGui::PushID(static_cast<int>(i));

            ImGui::SeparatorText(("Directional " + std::to_string(i)).c_str());

            glm::vec3 direction = light->getDirection();

            if (ImGui::DragFloat3("Direction", &direction.x, 0.01f, -1.0f, 1.0f)) {
                light->setDirection(direction);
            }

            glm::vec3 color = light->getColor();

            if (ImGui::ColorEdit3("Color", &color.x)) {
                light->setColor(color);
            }

            float intensity = light->getIntensity();

            if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 20.0f)) {
                light->setIntensity(intensity);
            }

            bool castsShadows = light->castsShadows();

            if (ImGui::Checkbox("Cast Shadows", &castsShadows)) {
                light->setCastsShadows(castsShadows);
            }

            ImGui::PopID();
        }
    }

    // =========================================================
    // POINT LIGHTS
    // =========================================================

    if (ImGui::CollapsingHeader("Point Lights", ImGuiTreeNodeFlags_DefaultOpen)) {

        for (size_t i = 0; i < pointLights.size(); i++) {

            auto* light = pointLights[i];

            ImGui::PushID(1000 + (int)i);

            ImGui::SeparatorText(("Point " + std::to_string(i)).c_str());

            auto position = light->getOwner()->transform()->getPosition();

            if (ImGui::DragFloat3("Position", &position.x, 0.05f)) {

                light->getOwner()->transform()->setPosition(position);
            }

            glm::vec3 color = light->getColor();
            if (ImGui::ColorEdit3("Color", &color.x)) {
                light->setColor(color);
            }

            float intensity = light->getIntensity();
            if (ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.0f, 100.0f)) {
                light->setIntensity(intensity);
            }

            float range = light->getRange();
            if (ImGui::DragFloat("Radius", &range, 0.1f, 0.1f, 500.0f)) {
                light->setRange(range);
            }

            ImGui::PopID();
        }
    }

    // =========================================================
    // SPOT LIGHTS
    // =========================================================

    if (ImGui::CollapsingHeader("Spot Lights", ImGuiTreeNodeFlags_DefaultOpen)) {

        for (size_t i = 0; i < spotLights.size(); i++) {

            auto* light = spotLights[i];

            ImGui::PushID(2000 + (int)i);

            ImGui::SeparatorText(("Spot " + std::to_string(i)).c_str());

            auto position = light->getOwner()->transform()->getPosition();

            if (ImGui::DragFloat3("Position", &position.x, 0.05f)) {

                light->getOwner()->transform()->setPosition(position);
            }

            glm::vec3 dir = light->getDirection();

            if (ImGui::DragFloat3("Direction", &dir.x, 0.01f, -1.0f, 1.0f)) {
                light->setDirection(dir);
            }

            glm::vec3 color = light->getColor();

            if (ImGui::ColorEdit3("Color", &color.x)) {
                light->setColor(color);
            }

            float intensity = light->getIntensity();

            if (ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.0f, 100.0f)) {
                light->setIntensity(intensity);
            }

            float innerAngle = glm::degrees(glm::acos(light->getInnerCutoff()));
            float outerAngle = glm::degrees(glm::acos(light->getOuterCutoff()));

            if (ImGui::DragFloat("Inner Cutoff", &innerAngle, 0.1f, 1.0f, 89.0f)) {

                float cosValue = glm::cos(glm::radians(innerAngle));
                light->setInnerCutoff(cosValue);
            }

            if (ImGui::DragFloat("Outer Cutoff", &outerAngle, 0.1f, 1.0f, 89.0f)) {

                float cosValue = glm::cos(glm::radians(outerAngle));
                light->setOuterCutoff(cosValue);
            }
            ImGui::PopID();
        }
    }

    ImGui::End();
}

#endif

void dzemikk::Renderer::setViewportSize(uint32_t width, uint32_t height) {

    if (width == 0 || height == 0) {
        return;
    }

    if (_viewportWidth == width && _viewportHeight == height) {
        return;
    }

    _viewportWidth = width;
    _viewportHeight = height;

    _sceneFramebuffer->resize(width, height);
}