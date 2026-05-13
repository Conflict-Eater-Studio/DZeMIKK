#include "inspectors/transformInspector.h"

#include "ui/propertyDrawer.h"
#include "ecs/components/transform.h"

#include <imgui.h>

using namespace dzemikk;

void editor::TransformInspector::draw(Transform* transform) {

    if (!transform) {
        return;
    }

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {

        glm::vec3 position = transform->getPosition();
        glm::vec3 rotation = transform->getEulerAngles();
        glm::vec3 scale = transform->getScale();

        if (PropertyDrawer::drawVec3("Position", position)) {
            transform->setPosition(position);
        }

        if (PropertyDrawer::drawVec3("Rotation", rotation)) {
            transform->setEulerAngles(rotation);
        }

        if (PropertyDrawer::drawVec3("Scale", scale, 1.0F)) {
            transform->setScale(scale);
        }
    }
}