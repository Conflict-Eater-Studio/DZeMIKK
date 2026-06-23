#include "renderer/renderPasses/uITextRenderPass.h"

#include "core/profiler.h"
#include "ecs/componentRegistry.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/gameobject.h"
#include "renderer/font.h"
#include "renderer/shader.h"

#include <GLFW/glfw3.h>
#include <limits>

dzemikk::UITextRenderPass::UITextRenderPass() {
    initTextShader();
}

void dzemikk::UITextRenderPass::execute(RenderContext& ctx) {

    ComponentRegistry::get().getEnabledComponents<UITextRenderer>(_texts);

    DZ_PROFILE_GPU("UI Text Rendering");

    for (auto* t : _texts) {

        if (!t->isValid())
            continue;

        auto* owner = t->getOwner();
        if (!owner)
            continue;

        auto* rect = owner->getComponent<RectTransform>();
        if (!rect)
            continue;

        const glm::mat4 world = rect->getWorldMatrix();
        const glm::vec4 p0 = world * glm::vec4(0, 0, 0, 1);

        const float originX = p0.x;
        const float originY = p0.y;

        const glm::vec2 rectSize = rect->getSize();

        float x = originX;
        float y = originY;

        float penX = 0.0f;
        float penY = 0.0f;

        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();

        const float lineHeight = t->font->lineHeight * t->scale;

        auto fallbackIt = t->font->characters.find('H');
        float fallbackAdvance =
            (fallbackIt != t->font->characters.end()) ? (fallbackIt->second.advance >> 6) : 10.0f;

        // =========================
        // PREPASS (UTF-8)
        // =========================
        for (size_t i = 0; i < t->text.size();) {

            unsigned int c = decodeUTF8(t->text, i);

            if (c == '\r')
                continue;

            if (c == '\n') {
                penY -= lineHeight;
                penX = 0.0f;
                continue;
            }

            if (c == ' ') {
                penX += fallbackAdvance * t->scale;
                continue;
            }

            auto it = t->font->characters.find(c);
            if (it == t->font->characters.end())
                continue;

            const Character& ch = it->second;

            float ypos = penY - (ch.size.y - ch.bearing.y) * t->scale;
            float h = ch.size.y * t->scale;

            minY = std::min(minY, ypos);
            maxY = std::max(maxY, ypos + h);

            penX += (ch.advance >> 6) * t->scale;
        }

        if (minY > maxY) {
            minY = 0.0f;
            maxY = 0.0f;
        }

        const float textWidth = penX;
        const float textHeight = maxY - minY;

        float offsetX = 0.0f;
        switch (t->horizontalAlign) {
        case UITextRenderer::HorizontalAlign::Left:
            offsetX = 0.0f;
            break;
        case UITextRenderer::HorizontalAlign::Center:
            offsetX = (rectSize.x - textWidth) * 0.5f;
            break;
        case UITextRenderer::HorizontalAlign::Right:
            offsetX = rectSize.x - textWidth;
            break;
        }

        float offsetY = 0.0f;
        switch (t->verticalAlign) {
        case UITextRenderer::VerticalAlign::Bottom:
            offsetY = -minY;
            break;
        case UITextRenderer::VerticalAlign::Middle:
            offsetY = (rectSize.y - textHeight) * 0.5f - minY;
            break;
        case UITextRenderer::VerticalAlign::Top:
            offsetY = rectSize.y - textHeight - minY;
            break;
        }

        Shader* shader = _textShader;
        shader->bind();

        shader->setMat4("projection", ctx.uiProjection);
        shader->setVec3("textColor", t->color);

        x = originX + offsetX;
        y = originY + offsetY;

        glBindVertexArray(_textVAO);

        // =========================
        // RENDER (UTF-8)
        // =========================
        for (size_t i = 0; i < t->text.size();) {

            unsigned int c = decodeUTF8(t->text, i);

            if (c == '\r')
                continue;

            if (c == '\n') {
                x = originX + offsetX;
                y -= lineHeight;
                continue;
            }

            if (c == ' ') {
                x += fallbackAdvance * t->scale;
                continue;
            }

            auto it = t->font->characters.find(c);
            if (it == t->font->characters.end())
                continue;

            const Character& ch = it->second;

            float xpos = x + ch.bearing.x * t->scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * t->scale;

            float w = ch.size.x * t->scale;
            float h = ch.size.y * t->scale;

            float vertices[6][4] = {
                {xpos, ypos + h, 0, 0}, {xpos, ypos, 0, 1},     {xpos + w, ypos, 1, 1},

                {xpos, ypos + h, 0, 0}, {xpos + w, ypos, 1, 1}, {xpos + w, ypos + h, 1, 0}};

            glBindTexture(GL_TEXTURE_2D, ch.textureID);

            glBindBuffer(GL_ARRAY_BUFFER, _textVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += (ch.advance >> 6) * t->scale;
        }

        glBindVertexArray(0);
    }
}

void dzemikk::UITextRenderPass::initTextShader() {
    const char* vertexSrc = R"(
    #version 460 core
    layout (location = 0) in vec4 vertex;

    out vec2 TexCoords;

    uniform mat4 projection;

    void main() {
        gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
        TexCoords = vertex.zw;
    }
    )";

    const char* fragmentSrc = R"(
    #version 460 core
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

unsigned int dzemikk::UITextRenderPass::decodeUTF8(const std::string& s, size_t& i) {
    unsigned char c = s[i];

    if (c < 0x80) {
        return s[i++];
    }

    if ((c >> 5) == 0x6) {
        unsigned int cp = ((s[i] & 0x1F) << 6) | (s[i + 1] & 0x3F);
        i += 2;
        return cp;
    }

    if ((c >> 4) == 0xE) {
        unsigned int cp = ((s[i] & 0x0F) << 12) | ((s[i + 1] & 0x3F) << 6) | (s[i + 2] & 0x3F);
        i += 3;
        return cp;
    }

    i++;
    return 0xFFFD;
}