#include "inspectors/gridLayoutInspector.h"
#include "ui/propertyDrawer.h"

#include <ecs/components/ui/gridLayout.h>

#include <imgui.h>

void editor::GridLayoutInspector::draw(dzemikk::GridLayout* layout, const InspectorContext& ctx) {
    if (!layout) {
        return;
    }

    if (ImGui::CollapsingHeader("GridLayout", ImGuiTreeNodeFlags_DefaultOpen)) {
        glm::vec2 cellSize = layout->getCellSize();
        glm::vec2 spacing = layout->getSpacing();

        int columns = layout->getColumns();

        auto startCorner = layout->getStartCorner();
        int currentCorner = static_cast<int>(startCorner);

        bool changed = false;

        if (PropertyDrawer::drawVec2("Cell Size", cellSize)) {
            layout->setCellSize(cellSize);
            changed = true;
        }

        if (PropertyDrawer::drawVec2("Spacing", spacing)) {
            layout->setSpacing(spacing);
            changed = true;
        }

        if (PropertyDrawer::drawInt("Columns", columns, 1)) {
            columns = std::max(1, columns);
            layout->setColumns(columns);
            changed = true;
        }

        const char* corners[] = {"Upper Left", "Upper Right", "Lower Left", "Lower Right"};

        if (PropertyDrawer::drawEnum("Start Corner", startCorner, corners, IM_ARRAYSIZE(corners))) {
            layout->setStartCorner(startCorner);
            changed = true;
        }
    }
}