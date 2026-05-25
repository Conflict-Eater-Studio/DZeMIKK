#include "inspectors/horizontalLayoutInspector.h"
#include "ui/propertyDrawer.h"

#include <ecs/components/ui/horizontalLayout.h>

#include <imgui.h>

void editor::HorizontalLayoutInspector::draw(dzemikk::HorizontalLayout* layout,
                                             const InspectorContext& ctx) {
    if (!layout) {
        return;
    }

    if (ImGui::CollapsingHeader("HorizontalLayout", ImGuiTreeNodeFlags_DefaultOpen)) {
        float spacing = layout->getSpacing();
        bool expandWidth = layout->getChildForceExpandWidth();
        bool expandHeight = layout->getChildForceExpandHeight();

        bool changed = false;

        if (PropertyDrawer::drawFloat("Spacing", spacing, 0.0F, 100.0F, 0.1F)) {
            layout->setSpacing(spacing);
            changed = true;
        }

        if (PropertyDrawer::drawBool("Force Expand Width", expandWidth)) {
            layout->setChildForceExpandWidth(expandWidth);
            changed = true;
        }

        if (PropertyDrawer::drawBool("Force Expand Height", expandHeight)) {
            layout->setChildForceExpandHeight(expandHeight);
            changed = true;
        }

        if (changed) {
            layout->rebuild();
        }
    }
}