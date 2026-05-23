#include "inspectors/canvasInspector.h"
#include "ui/propertyDrawer.h"
#include "ecs/components/ui/canvas.h"
#include "inspectors/inspectorRegistry.h"
#include <imgui.h>

void editor::CanvasInspector::draw(dzemikk::Canvas* canvas, const InspectorContext& ctx) {
    if (!canvas) {
        return;
    }

    if (ImGui::CollapsingHeader("Canvas", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextDisabled("No editable properties");
    }

}
