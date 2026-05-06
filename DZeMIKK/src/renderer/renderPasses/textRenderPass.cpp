#include "renderer/renderPasses/textRenderPass.h"
#include "renderer/shader.h"
#include "renderer/font.h"

#include "ecs/componentRegistry.h"
#include "ecs/components/textRenderer.h"
#include "ecs/gameobject.h"

#include "core/profiler.h"

#include <GLFW/glfw3.h>

dzemikk::TextRenderPass::TextRenderPass() {
    initTextShader();
}

void dzemikk::TextRenderPass::execute(RenderContext& ctx) {
    ComponentRegistry::get().getComponents<TextRenderer>(_texts);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    {
        DZ_PROFILE_GPU("Text Rendering");
        for (auto* t : _texts) {
            if (!t->isValid())
                continue;

            Shader* shader = _textShader;
            shader->bind();

            shader->setMat4("projection", ctx.uiProjection);
            shader->setVec3("textColor", t->color);

            float x = t->getOwner()->transform()->getPosition().x;
            float y = t->getOwner()->transform()->getPosition().y;

            glBindVertexArray(_textVAO);
            for (char c : t->text) {
                Character ch = t->font.get()->characters[c];

                float xpos = x + ch.bearing.x * t->scale;
                float ypos = y - (ch.size.y - ch.bearing.y) * t->scale;

                float w = ch.size.x * t->scale;
                float h = ch.size.y * t->scale;

                float vertices[6][4] = {
                    {xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
                    {xpos + w, ypos, 1.0f, 1.0f},

                    {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
                    {xpos + w, ypos + h, 1.0f, 0.0f}};

                glBindTexture(GL_TEXTURE_2D, ch.textureID);

                glBindBuffer(GL_ARRAY_BUFFER, _textVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

                glDrawArrays(GL_TRIANGLES, 0, 6);

                Profiler::Get().stats.drawCalls++;

                Profiler::Get().stats.renderedObjects++;
                Profiler::Get().stats.vertexCount += 6;
                Profiler::Get().stats.triangleCount += 2;

                x += (ch.advance >> 6) * t->scale;
            }
            glBindVertexArray(0);
        }
    }
}

void dzemikk::TextRenderPass::initTextShader() {
    const char* vertexSrc = R"(
    #version 330 core
    layout (location = 0) in vec4 vertex; // pos.xy, uv.xy

    out vec2 TexCoords;

    uniform mat4 projection;

    void main() {
        gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
        TexCoords = vertex.zw;
    }
    )";

    const char* fragmentSrc = R"(
    #version 330 core
    in vec2 TexCoords;
    out vec4 color;

    uniform sampler2D text;
    uniform vec3 textColor;

    void main() {
        float alpha = texture(text, TexCoords).r;
        color = vec4(textColor, alpha);
    }
    )";

    _textShader = new Shader(vertexSrc, fragmentSrc);
    _textShader->uploadToGPU();

    glGenVertexArrays(1, &_textVAO);
    glGenBuffers(1, &_textVBO);

    glBindVertexArray(_textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
