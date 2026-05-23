#include "inspectors/spotLightInspector.h"
#include "ui/propertyDrawer.h"
#include "ecs/components/light/spotLight.h"
#include "inspectors/inspectorRegistry.h"
#include <imgui.h>

void editor::SpotLightInspector::draw(dzemikk::SpotLight* light, const InspectorContext& ctx) {
    if (!light) {
        return;
    }

    if (ImGui::CollapsingHeader("SpotLight", ImGuiTreeNodeFlags_DefaultOpen)) {
        glm::vec3 direction = light->getDirection();
        glm::vec4 color = glm::vec4(light->getColor().x, light->getColor().y, light->getColor().z, 1);
        float intensity = light->getIntensity();
        float innerCutOff = light->getInnerCutoff();
        float outerCutOff = light->getOuterCutoff();
        float range = light->getRange();
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

        if (PropertyDrawer::drawFloat("InnerCutOff", innerCutOff)) {
            light->setInnerCutoff(innerCutOff);
        }

        if (PropertyDrawer::drawFloat("OuterCutOff", outerCutOff)) {
            light->setOuterCutoff(outerCutOff);
        }

        if (PropertyDrawer::drawFloat("Range", range)) {
            light->setRange(range);
        }

        if (PropertyDrawer::drawBool("CastShadows", castsShadows)) {
            light->setCastsShadows(castsShadows);
        }
    }
}
