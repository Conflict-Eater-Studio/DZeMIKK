#include "inspectors/uiTextRendererInspector.h"
#include "inspectors/inspectorRegistry.h"
#include "ui/propertyDrawer.h"

#include <ecs/components/ui/uiTextRenderer.h>

#include <imgui.h>

void editor::UITextRendererInspector::draw(dzemikk::UITextRenderer* textRenderer,
                                           const InspectorContext& ctx) {
    if (!textRenderer) {
        return;
    }

    if (ImGui::CollapsingHeader("UITextRenderer", ImGuiTreeNodeFlags_DefaultOpen)) {
        static std::unordered_map<void*, std::array<char, 1024>> textBuffers;

        auto& textBuffer = textBuffers[textRenderer];

        std::snprintf(textBuffer.data(), textBuffer.size(), "%s", textRenderer->text.c_str());

        if (ImGui::InputTextMultiline("Text", textBuffer.data(), textBuffer.size())) {
            textRenderer->text = textBuffer.data();
        }

        float scale = textRenderer->scale;

        if (PropertyDrawer::drawFloat("Scale", scale, 0.01F)) {
            textRenderer->scale = scale;
        }

        glm::vec4 color = glm::vec4(textRenderer->color, 1.0F);

        if (PropertyDrawer::drawColor("Color", color)) {
            textRenderer->color = glm::vec3(color);
        }

        const char* horizontalItems[] = {"Right", "Center", "Left"};

        auto horizontal = textRenderer->horizontalAlign;

        if (PropertyDrawer::drawEnum("Horizontal Align", horizontal, horizontalItems,
                                     IM_ARRAYSIZE(horizontalItems))) {
            textRenderer->horizontalAlign = horizontal;
        }

        const char* verticalItems[] = {"Bottom", "Middle", "Top"};

        auto vertical = textRenderer->verticalAlign;

        if (PropertyDrawer::drawEnum("Vertical Align", vertical, verticalItems,
                                     IM_ARRAYSIZE(verticalItems))) {
            textRenderer->verticalAlign = vertical;
        }

        auto fontHandle = textRenderer->fontAsset;

        if (PropertyDrawer::drawFont("Font", fontHandle, ctx)) {
            textRenderer->fontAsset = fontHandle;
            textRenderer->font = fontHandle.get();
        }
    }
}