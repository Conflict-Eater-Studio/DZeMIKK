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

void dzemikk::Renderer::initialize() {
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
    setupFrame();
    _cameraSystem.update(_context);
    _context.sceneCamera = _cameraSystem.getActiveSceneCamera();
    _context.uiCamera = _cameraSystem.getActiveUICamera();

    for (auto& pass : _passes)
        pass->execute(_context);
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}