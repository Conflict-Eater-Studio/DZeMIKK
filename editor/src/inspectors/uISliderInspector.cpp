#include "inspectors/uiSliderInspector.h"
#include "inspectors/inspectorRegistry.h"
#include "ui/propertyDrawer.h"

#include <ecs/components/ui/uiSlider.h>

#include <imgui.h>

void editor::UISliderInspector::draw(dzemikk::UISlider* slider, const InspectorContext& ctx) {
    if (!slider) {
        return;
    }

    if (ImGui::CollapsingHeader("UI Slider", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto style = slider->getStyle();

        bool changed = false;

        changed |= PropertyDrawer::drawColor("Fill Color", style.fillColor);
        changed |= PropertyDrawer::drawColor("Background Color", style.backgroundColor);
        changed |= PropertyDrawer::drawColor("Handle Color", style.handleColor);
        changed |= PropertyDrawer::drawColor("Handle Hover Color", style.handleHoverColor);
        changed |= PropertyDrawer::drawColor("Handle Pressed Color", style.handlePressedColor);

        float value = slider->getValue();
        float minValue = slider->getMinValue();
        float maxValue = slider->getMaxValue();
        float step = slider->getStep();

        changed |= ImGui::DragFloat("Value", &value, step, minValue, maxValue);
        changed |= ImGui::DragFloat("Min Value", &minValue, 0.1F);
        changed |= ImGui::DragFloat("Max Value", &maxValue, 0.1F);
        changed |= ImGui::DragFloat("Step", &step, 0.001F, 0.0001F, 1.0F);

        if (changed) {
            slider->setStyle(style);

            slider->setMinValue(minValue);
            slider->setMaxValue(maxValue);
            slider->setStep(step);

            slider->onValueChanged(value);

            slider->applyVisualState();
        }
        
        ImGui::Spacing();
        ImGui::Separator();

        PropertyDrawer::drawUIEvents("Events", slider, ctx);
    }
}