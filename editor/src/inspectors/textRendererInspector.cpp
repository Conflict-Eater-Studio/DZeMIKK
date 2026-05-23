#include "inspectors/textRendererInspector.h"
#include "inspectors/inspectorRegistry.h"

#include "ecs/components/textRenderer.h"
#include "ui/propertyDrawer.h"

#include <imgui.h>

void editor::TextRendererInspector::draw(dzemikk::TextRenderer* textRenderer,
                                         const InspectorContext& ctx) {

    if (!textRenderer) {
        return;
    }

    if (ImGui::CollapsingHeader("TextRenderer", ImGuiTreeNodeFlags_DefaultOpen)) {

        char buffer[1024];

        std::snprintf(buffer, sizeof(buffer), "%s", textRenderer->text.c_str());

        if (ImGui::InputTextMultiline("Text", buffer, sizeof(buffer))) {
            textRenderer->text = buffer;
        }

        float scale = textRenderer->scale;

        if (PropertyDrawer::drawFloat("Scale", scale, 0.01f)) {
            textRenderer->scale = scale;
        }

        glm::vec4 color = glm::vec4(textRenderer->color, 1.0f);

        if (PropertyDrawer::drawColor("Color", color)) {
            textRenderer->color = glm::vec3(color);
        }

        auto fontHandle = textRenderer->font;

        if (PropertyDrawer::drawFont("Font", fontHandle, ctx)) {
            textRenderer->font = fontHandle;
        }
    }
}