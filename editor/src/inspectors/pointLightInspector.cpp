#include "inspectors/pointLightInspector.h"
#include "inspectors/inspectorRegistry.h"
#include "ui/propertyDrawer.h"

#include <ecs/components/light/pointLight.h>

#include <imgui.h>

void editor::PointLightInspector::draw(dzemikk::PointLight* light, const InspectorContext& ctx) {
    if (!light) {
        return;
    }

    if (ImGui::CollapsingHeader("PointLight", ImGuiTreeNodeFlags_DefaultOpen)) {
        glm::vec4 color =
            glm::vec4(light->getColor().x, light->getColor().y, light->getColor().z, 1);
        float intensity = light->getIntensity();
        float range = light->getRange();
        bool castsShadows = light->castsShadows();

        if (PropertyDrawer::drawColor("Color", color)) {
            light->setColor(color);
        }

        if (PropertyDrawer::drawFloat("Intensity", intensity, 0.0F, 10.0F, 0.1F)) {
            light->setIntensity(intensity);
        }

        if (PropertyDrawer::drawFloat("Range", range)) {
            light->setRange(range);
        }

        if (PropertyDrawer::drawBool("CastShadows", castsShadows)) {
            light->setCastsShadows(castsShadows);
        }
    }

}
