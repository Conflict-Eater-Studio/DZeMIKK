#include "renderer/renderPasses/bloomRenderPass.h"

#include <glm/glm.hpp>
#include <assetManager/assetmanager.h>
#include <core/engine.h>

namespace dzemikk {

BloomRenderPass::BloomRenderPass(){
    initializeQuad();

    _brightFramebuffer = std::make_unique<Framebuffer>(1920, 1080);
    _pingFramebuffer = std::make_unique<Framebuffer>(1920, 1080);
    _pongFramebuffer = std::make_unique<Framebuffer>(1920, 1080);
}

BloomRenderPass::~BloomRenderPass() {
    destroyQuad();
}

void BloomRenderPass::initializeQuad() {
    float vertices[] = {-1.f, 1.f, 0.f, 1.f, -1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 1.f, 0.f,

                        -1.f, 1.f, 0.f, 1.f, 1.f,  -1.f, 1.f, 0.f, 1.f, 1.f,  1.f, 1.f};

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void BloomRenderPass::destroyQuad() {
    if (_vbo) {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }

    if (_vao) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
}

void BloomRenderPass::initialize() {
    AssetManager* assetManager = _engine->getAssetManager();
    _brightShader = assetManager->get<Shader>("shaders/Bloom/brightPass");
    _blurShader = assetManager->get<Shader>("shaders/Bloom/blurPass");
    _compositeShader = assetManager->get<Shader>("shaders/Bloom/compositePass");
}

void BloomRenderPass::execute(RenderContext& ctx) {
    if (!_brightShader.get() || !_blurShader.get() || !_compositeShader.get()) {
        return;
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    GLuint sceneTexture = ctx.sceneTexture;

    _brightFramebuffer->bind();

    glViewport(0, 0, _brightFramebuffer->getWidth(), _brightFramebuffer->getHeight());

    glClear(GL_COLOR_BUFFER_BIT);

    _brightShader.get()->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);

    _brightShader.get()->setSampler("screenTexture", 0);
    _brightShader.get()->setFloat("threshold", 0.7f);

    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    constexpr int blurIterations = 8;

    bool horizontal = true;
    bool firstIteration = true;

    _blurShader.get()->bind();

    for (int i = 0; i < blurIterations; ++i) {

        Framebuffer* target = horizontal ? _pingFramebuffer.get() : _pongFramebuffer.get();

        target->bind();

        glViewport(0, 0, target->getWidth(), target->getHeight());

        glClear(GL_COLOR_BUFFER_BIT);

        GLuint sourceTexture;

        if (firstIteration) {
            sourceTexture = _brightFramebuffer->getColorAttachmentRendererID();
        } else {
            sourceTexture = horizontal ? _pongFramebuffer->getColorAttachmentRendererID()
                                       : _pingFramebuffer->getColorAttachmentRendererID();
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sourceTexture);

        _blurShader.get()->setSampler("image", 0);
        _blurShader.get()->setBool("horizontal", horizontal);

        glBindVertexArray(_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        firstIteration = false;
        horizontal = !horizontal;
    }

    GLuint bloomTexture = horizontal ? _pingFramebuffer->getColorAttachmentRendererID()
                                     : _pongFramebuffer->getColorAttachmentRendererID();

    Framebuffer* output = horizontal ? _pongFramebuffer.get() : _pingFramebuffer.get();

    output->bind();

    glViewport(0, 0, output->getWidth(), output->getHeight());

    glClear(GL_COLOR_BUFFER_BIT);

    _compositeShader.get()->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);

    _compositeShader.get()->setSampler("sceneTexture", 0);
    _compositeShader.get()->setSampler("bloomTexture", 1);
    _compositeShader.get()->setFloat("intensity", 0.2f);

    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);

    ctx.sceneTexture = output->getColorAttachmentRendererID();
}

} // namespace dzemikk