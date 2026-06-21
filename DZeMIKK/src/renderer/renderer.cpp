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
#include "renderer/renderPasses/BloomRenderPass.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <map>

#if DZEMIKK_DEV_TOOLS
#include <imgui.h>
#endif

#include "core/engine.h"
#include "assetManager/assetmanager.h"

void dzemikk::Renderer::initialize() {
    _sceneFramebuffer = std::make_unique<Framebuffer>(_viewportWidth, _viewportHeight);
    _lightSSBO.init();

    _context = RenderContext(_cameraSystem.getActiveSceneCamera(),
                             _cameraSystem.getActiveUICamera(), glm::mat4(1.0f),
                             glm::mat4(1.0f),
                                 glm::ortho(0.0f, 1920.0f, 0.0f, 1080.0f), _uboMatrices, &_frustum);

    addPass<SkyboxRenderPass>();
    addPass<MeshRenderPass>();
    addPass<SkinnedRenderPass>();
    addPass<SpriteRenderPass>();

    addUIPass<ImageRenderPass>();
    addUIPass<TextRenderPass>();
    addUIPass<UITextRenderPass>();

    _bloomRenderPass.setEngine(_engine);
    _bloomRenderPass.initialize();

    glGenBuffers(1, &_uboMatrices);

    glBindBuffer(GL_UNIFORM_BUFFER, _uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, _uboMatrices, 0, 2 * sizeof(glm::mat4));

    glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _context.uboMatrices = _uboMatrices;

    initFullscreenQuad();
    _presentShader = _engine->getAssetManager()->get<Shader>("shaders/blit");
}

void dzemikk::Renderer::uninitialize() {
    if (_uboMatrices != 0) {
        glDeleteBuffers(1, &_uboMatrices);
        _uboMatrices = 0;
    }

    _lightSSBO.destroy();
}

void dzemikk::Renderer::render() {
    _lightSystem.update(_context);
    _lightSSBO.upload(_lightSystem);

    _sceneFramebuffer->bind();
    glViewport(0, 0, _viewportWidth, _viewportHeight);

    setupFrame();
    _cameraSystem.update(_context);
    _context.sceneCamera = _cameraSystem.getActiveSceneCamera();
    _context.uiCamera = _cameraSystem.getActiveUICamera();

    for (auto& pass : _passes) {
        pass->execute(_context);
    }

    if (_engineMode == EngineMode::Editor) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (auto& pass : _uiPasses) {
            pass->execute(_context);
        }
    }

    _context.sceneTexture = _sceneFramebuffer->getColorAttachmentRendererID();

    _sceneFramebuffer->unbind();

    _bloomRenderPass.execute(_context);
    _postProcessingPass.execute(_context);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, _viewportWidth, _viewportHeight);

    glDisable(GL_DEPTH_TEST);

    _presentShader.get()->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _context.sceneTexture);

    _presentShader.get()->setSampler("screenTexture", 0);

    glBindVertexArray(_fullscreenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);

    if (_engineMode == EngineMode::Game) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (auto& pass : _uiPasses) {
            pass->execute(_context);
        }
    }
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
    glViewport(0, 0, _viewportWidth, _viewportHeight);    

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

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

void dzemikk::Renderer::initFullscreenQuad() {
    float vertices[] = {// pos   // uv
                        -1.f, 1.f, 0.f, 1.f, -1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 1.f, 0.f,

                        -1.f, 1.f, 0.f, 1.f, 1.f,  -1.f, 1.f, 0.f, 1.f, 1.f,  1.f, 1.f};

    glGenVertexArrays(1, &_fullscreenVAO);
    glBindVertexArray(_fullscreenVAO);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}