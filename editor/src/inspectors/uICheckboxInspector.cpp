#include "inspectors/uiCheckboxInspector.h"

#include "ecs/components/ui/uiCheckbox.h"
#include "ecs/components/ui/iUIInteractable.h" 
#include "ui/propertyDrawer.h"

#include <imgui.h>

using namespace dzemikk;

void editor::UICheckboxInspector::draw(dzemikk::UICheckbox* checkbox, const InspectorContext& ctx) {
    if (!checkbox) {
        return;
    }

    if (ImGui::CollapsingHeader("UI Checkbox", ImGuiTreeNodeFlags_DefaultOpen)) {

        auto style = checkbox->getStyle();
        bool changed = false;

        changed |= PropertyDrawer::drawColor("Normal Color", style.normalColor);
        changed |= PropertyDrawer::drawColor("Hover Color", style.hoverColor);
        changed |= PropertyDrawer::drawColor("Pressed Color", style.pressedColor);
        changed |= PropertyDrawer::drawColor("Checkmark Color", style.checkmarkColor);

        if (changed) {
            checkbox->setStyle(style);
            checkbox->applyVisualState();
        }

        bool value = checkbox->getValue();
        if (ImGui::Checkbox("Value", &value)) {
            checkbox->setValue(value);
        }


        ImGui::Spacing();
        ImGui::Separator();

        PropertyDrawer::drawUIEvents("Events", checkbox, ctx);
    }
}