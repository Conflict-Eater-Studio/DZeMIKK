#include "inspectors/directionalLightInspector.h"
#include "ecs/components/light/directionalLight.h"
#include "inspectors/inspectorRegistry.h"

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
        glm::vec4 color = glm::vec4(light->getColor().x, light->getColor().y, light->getColor().z, 1);
        float intensity = light->getIntensity();
        bool castsShadows = light->castsShadows();

        if (PropertyDrawer::drawVec3("Direction", direction)) {
            light->setDirection(direction);
        }

        if (PropertyDrawer::drawColor("Color", color)) {
            light->setColor(color);
        }

        if (PropertyDrawer::drawFloat("Intensity", intensity, 0.0F, 10.0F, 0.1F)) {
            light->setIntensity(intensity);
        }

        if (PropertyDrawer::drawBool("CastShadows", castsShadows)) {
            light->setCastsShadows(castsShadows);
        }
    }
}
