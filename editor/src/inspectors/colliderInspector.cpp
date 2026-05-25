#include "inspectors/colliderInspector.h"

#include <ecs/components/collider.h>
#include <ui/propertyDrawer.h>

#include <imgui.h>

void editor::ColliderInspector::draw(dzemikk::Collider* collider, const InspectorContext& ctx) {
    if (!collider) {
        return;
    }

    if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextDisabled("No editable properties");
    }
}