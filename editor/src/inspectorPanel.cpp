#include "inspectorPanel.h"

#include "ecs/components/transform.h"

#include "inspectors/transformInspector.h"

#include <imgui.h>

void editor::InspectorPanel::draw(dzemikk::GameObject* selectedObject) {

    ImGui::Begin("Inspector");

    if (!selectedObject) {
        ImGui::Text("No object selected");
        ImGui::End();
        return;
    }

    ImGui::Text("%s", selectedObject->getName().c_str());

    TransformInspector::draw(selectedObject->transform());

    ImGui::End();
}