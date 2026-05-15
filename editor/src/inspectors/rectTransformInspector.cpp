#include "inspectors/rectTransformInspector.h"
#include "ui/propertyDrawer.h"

#include <imgui.h>

void editor::RectTransformInspector::draw(dzemikk::RectTransform* transform,
                                          const InspectorContext& ctx) {
    if (!transform) {
        return;
    }

    if (ImGui::CollapsingHeader("RectTransform", ImGuiTreeNodeFlags_DefaultOpen)) {
        glm::vec2 position = transform->getPosition();
        glm::vec2 size = transform->getSize();
        glm::vec2 scale = transform->getScale();

        float rotation = transform->getRotation();

        glm::vec2 pivot = transform->getPivot();

        glm::vec2 anchorMin = transform->getAnchorMin();
        glm::vec2 anchorMax = transform->getAnchorMax();

        glm::vec2 offsetMin = transform->getOffsetMin();
        glm::vec2 offsetMax = transform->getOffsetMax();

        int zIndex = transform->getZIndex();
        
        if (PropertyDrawer::drawVec2("Position", position)) {
            transform->setPosition(position);
        }

        if (PropertyDrawer::drawVec2("Size", size)) {
            transform->setSize(size);
        }

        if (PropertyDrawer::drawVec2("Scale", scale)) {
            transform->setScale(scale);
        }

        if (PropertyDrawer::drawFloat("Rotation", rotation)) {
            transform->setRotation(rotation);
        }

        if (PropertyDrawer::drawVec2("Pivot", pivot)) {
            transform->setPivot(pivot);
        }

        if (PropertyDrawer::drawVec2("Anchor Min", anchorMin)) {
            transform->setAnchorMin(anchorMin);
        }

        if (PropertyDrawer::drawVec2("Anchor Max", anchorMax)) {
            transform->setAnchorMax(anchorMax);
        }

        if (PropertyDrawer::drawVec2("Offset Min", offsetMin)) {
            transform->setOffsetMin(offsetMin);
        }

        if (PropertyDrawer::drawVec2("Offset Max", offsetMax)) {
            transform->setOffsetMax(offsetMax);
        }

        if (PropertyDrawer::drawInt("Z Index", zIndex)) {
            transform->setZIndex(static_cast<unsigned int>(zIndex));
        }

    }
}
