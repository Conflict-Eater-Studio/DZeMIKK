#include "inspectors/uIButtonInspector.h"
#include "ui/propertyDrawer.h"

#include <imgui.h>

void editor::UIButtonInspector::draw(dzemikk::UIButton* uiButton, const InspectorContext& ctx) {
    if (!uiButton) {
        return;
    }

    if (ImGui::CollapsingHeader("UIButton", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextDisabled("No editable properties");
    }
}
