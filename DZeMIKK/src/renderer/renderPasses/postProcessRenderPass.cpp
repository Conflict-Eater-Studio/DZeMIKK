#include "renderer/renderPasses/postProcessRenderPass.h"

#include "ecs/componentRegistry.h"
#include "ecs/components/postProcessEffect.h"

#include "renderer/shader.h"
#include <glm/glm.hpp>

dzemikk::PostProcessRenderPass::PostProcessRenderPass() {
    initializeQuad();

    _pingFramebuffer = std::make_unique<Framebuffer>(1920, 1080);
    _pongFramebuffer = std::make_unique<Framebuffer>(1920, 1080);
}

dzemikk::PostProcessRenderPass::~PostProcessRenderPass() {
    destroyQuad();
}

void dzemikk::PostProcessRenderPass::initializeQuad() {
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

void dzemikk::PostProcessRenderPass::destroyQuad() {
    if (_vbo)
        glDeleteBuffers(1, &_vbo);

    if (_vao)
        glDeleteVertexArrays(1, &_vao);
}

void dzemikk::PostProcessRenderPass::execute(RenderContext& ctx) {

    ComponentRegistry::get().getEnabledComponents<PostProcessEffect>(_effects);

    if (_effects.empty()) {
        return;
    }

    std::sort(_effects.begin(), _effects.end(),
              [](auto* a, auto* b) { return a->getPriority() < b->getPriority(); });

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    GLuint inputTexture = ctx.sceneTexture;

    bool usePing = true;

    for (auto* effect : _effects) {

        if (!effect->isEnabled())
            continue;

        Shader* shader = effect->getShader().get();

        if (!shader)
            continue;

        Framebuffer* target = usePing ? _pingFramebuffer.get() : _pongFramebuffer.get();

        target->bind();

        glViewport(0, 0, target->getWidth(), target->getHeight());

        glClear(GL_COLOR_BUFFER_BIT);

        shader->bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputTexture);

        shader->setSampler("screenTexture", 0);

        shader->setVec2("uScreenSize", glm::vec2(target->getWidth(), target->getHeight()));
        shader->setVec2("uInverseScreenSize", glm::vec2(1.0f / target->getWidth(), 1.0f / target->getHeight()));

        effect->bindShaderUniforms(*shader);

        glBindVertexArray(_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        inputTexture = target->getColorAttachmentRendererID();

        usePing = !usePing;
    }

    ctx.sceneTexture = inputTexture;
}