#include "inspectors/uIButtonInspector.h"

#include "ecs/components/ui/imageRenderer.h"
#include "ecs/gameObject.h"
#include "ui/propertyDrawer.h"

#include <imgui.h>

void editor::UIButtonInspector::draw(dzemikk::UIButton* uiButton, const InspectorContext& ctx) {
    if (!uiButton) {
        return;
    }

    if (ImGui::CollapsingHeader("UIButton", ImGuiTreeNodeFlags_DefaultOpen)) {

        bool changed = false;

        auto style = uiButton->getStyle();

        ImGui::SeparatorText("Style");

        changed |= editor::PropertyDrawer::drawColor("Normal Color", style.normalColor);

        changed |= editor::PropertyDrawer::drawColor("Hover Color", style.hoverColor);

        changed |= editor::PropertyDrawer::drawColor("Pressed Color", style.pressedColor);

        if (changed) {
            uiButton->setStyle(style);
        }

        ImGui::SeparatorText("References");

        auto* spriteRenderer = uiButton->getSpriteRenderer();

        ImGui::Text("Sprite Renderer: %s", spriteRenderer ? "Attached" : "Missing");

        auto* textGO = uiButton->getTextGO();

        if (textGO) {
            ImGui::Text("Text GameObject: %s", textGO->getName().c_str());
        } else {
            ImGui::Text("Text GameObject: null");
        }
    }
}