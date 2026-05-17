#include "renderer/renderPasses/uITextRenderPass.h"

#include "core/profiler.h"
#include "ecs/componentRegistry.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/gameobject.h"
#include "renderer/font.h"
#include "renderer/shader.h"

#include <GLFW/glfw3.h>

dzemikk::UITextRenderPass::UITextRenderPass() {
    initTextShader();
}

void dzemikk::UITextRenderPass::execute(RenderContext& ctx) {

    ComponentRegistry::get().getEnabledComponents<UITextRenderer>(_texts);

    {
        DZ_PROFILE_GPU("UI Text Rendering");
        for (auto* t : _texts) {
            if (!t->isValid()) {
                continue;
            }

            auto* owner = t->getOwner();
            if (!owner) {
                continue;
            }

            auto* rect = owner->getComponent<RectTransform>();
            if (!rect) {
                continue;
            }

            const glm::mat4 world = rect->getWorldMatrix();
            const glm::vec4 p0 = world * glm::vec4(0.0F, 0.0F, 0.0F, 1.0F);
            const float originX = p0[0];
            const float originY = p0[1];
            const glm::vec2 rectSize = rect->getSize();

            float penX = 0.0F;
            float minY = std::numeric_limits<float>::max();
            float maxY = std::numeric_limits<float>::lowest();

            for (char c : t->text) {
                auto glyphIt = t->font->characters.find(c);
                if (glyphIt == t->font->characters.end()) {
                    continue;
                }

                const Character& ch = glyphIt->second;
                const float ypos = -(ch.size.y - ch.bearing.y) * t->scale;
                const float h = ch.size.y * t->scale;

                minY = std::min(minY, ypos);
                maxY = std::max(maxY, ypos + h);

                penX += (ch.advance >> 6) * t->scale;
            }

            if (minY > maxY) {
                minY = 0.0F;
                maxY = 0.0F;
            }

            const float textWidth = penX;
            const float textHeight = maxY - minY;

            float offsetX = 0.0F;
            switch (t->horizontalAlign) {
            case UITextRenderer::HorizontalAlign::Left:
                offsetX = 0.0F;
                break;
            case UITextRenderer::HorizontalAlign::Center:
                offsetX = (rectSize.x - textWidth) * 0.5F;
                break;
            case UITextRenderer::HorizontalAlign::Right:
                offsetX = rectSize.x - textWidth;
                break;
            }

            float offsetY = 0.0F;
            switch (t->verticalAlign) {
            case UITextRenderer::VerticalAlign::Bottom:
                offsetY = -minY;
                break;
            case UITextRenderer::VerticalAlign::Middle:
                offsetY = (rectSize.y - textHeight) * 0.5F - minY;
                break;
            case UITextRenderer::VerticalAlign::Top:
                offsetY = rectSize.y - textHeight - minY;
                break;
            }

            Shader* shader = _textShader;
            shader->bind();

            shader->setMat4("projection", ctx.uiProjection);
            shader->setVec3("textColor", t->color);

            float x = originX + offsetX;
            float y = originY + offsetY;

            glBindVertexArray(_textVAO);
            for (char c : t->text) {
                auto glyphIt = t->font->characters.find(c);
                if (glyphIt == t->font->characters.end()) {
                    continue;
                }

                const Character& ch = glyphIt->second;

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

void dzemikk::UITextRenderPass::initTextShader() {
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
