#include "inspectorPanel.h"

#include "ecs/components/transform.h"

#include "inspectors/transformInspector.h"
#include "inspectors/meshRendererInspector.h"

#include <imgui.h>
#include <ecs/components/meshRenderer.h>

editor::InspectorPanel::InspectorPanel() {

    _registry.registerInspector("Transform",
                                [](dzemikk::Component* component, const InspectorContext&) {
                                    auto* transform = dynamic_cast<dzemikk::Transform*>(component);
                                    if (!transform)
                                        return;

                                    TransformInspector::draw(transform);
                                });

    _registry.registerInspector(
        "MeshRenderer", [](dzemikk::Component* component, const InspectorContext& ctx) {
            auto* renderer = dynamic_cast<dzemikk::MeshRenderer*>(component);
            if (!renderer)
                return;

            MeshRendererInspector::draw(renderer, ctx);
        });
}

void editor::InspectorPanel::draw(dzemikk::GameObject* selectedObject,
                                  const InspectorContext& context) {

    ImGui::Begin("Inspector");

    if (!selectedObject) {
        ImGui::Text("No object selected");
        ImGui::End();
        return;
    }

    ImGui::Text("%s", selectedObject->getName().c_str());

    for (auto& component : selectedObject->getAllComponents()) {
        _registry.drawInspector(component.get(), context);
    }

    ImGui::End();
}