#include "inspectors/directionalLightInspector.h"

#include "ui/propertyDrawer.h"

#include <imgui.h>

using namespace dzemikk;

void editor::DirectionalLightInspector::draw(dzemikk::DirectionalLight* light,
                                             const InspectorContext& ctx) {
    if (!light) {
        return;
    }

    if (ImGui::CollapsingHeader("DirectionalLight", ImGuiTreeNodeFlags_DefaultOpen)) {

        glm::vec3 direction = light->getDirection();
        glm::vec4 color = glm::vec4(light->getColor().x, light->getColor().y, light->getColor().z, 0);
        float intensity = light->getIntensity();
        bool castsShadows = light->castsShadows();

        if (PropertyDrawer::drawVec3("Direction", direction)) {
            light->setDirection(direction);
        }

        if (PropertyDrawer::drawColor("Color", color)) {
            light->setColor(color);
        }

        if (PropertyDrawer::drawFloat("Intensity", intensity)) {
            light->setIntensity(intensity);
        }

        if (PropertyDrawer::drawBool("CastShadows", castsShadows)) {
            light->setCastsShadows(castsShadows);
        }
    }
}
