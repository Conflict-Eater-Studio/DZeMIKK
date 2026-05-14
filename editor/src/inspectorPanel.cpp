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



    _factories.push_back({"Transform", [](dzemikk::GameObject* go) {
                              if (!go->getComponent<dzemikk::Transform>())
                                  go->addComponent<dzemikk::Transform>();
                          }});

    _factories.push_back({"MeshRenderer", [](dzemikk::GameObject* go) {
                              if (!go->getComponent<dzemikk::MeshRenderer>()) {
                                  auto renderer = go->addComponent<dzemikk::MeshRenderer>();
                                  renderer->setTransform(go->getComponent<dzemikk::Transform>());
                              }

                          }});
}

void editor::InspectorPanel::draw(dzemikk::GameObject* selectedObject,
                                  const InspectorContext& context) {

    ImGui::Begin("Inspector");

    if (!selectedObject) {
        ImGui::Text("No object selected");
        ImGui::End();
        return;
    }

    static char nameBuffer[256];

    strncpy(nameBuffer, selectedObject->getName().c_str(), sizeof(nameBuffer));
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';

    if (ImGui::InputText("##name", nameBuffer, sizeof(nameBuffer))) {
        selectedObject->setName(nameBuffer);
    }

    bool enabled = selectedObject->isEnabled();

    if (ImGui::Checkbox("Enabled", &enabled)) {
        selectedObject->enabled(enabled);
    }

    ImGui::Separator();

    ImGui::Spacing();
    ImGui::Text("Components");
    ImGui::Spacing();

    ImGui::Spacing();

    for (auto& component : selectedObject->getAllComponents()) {
        ImGui::PushID(component.get());

        bool isTransform = dynamic_cast<dzemikk::Transform*>(component.get()) != nullptr;

        ImGui::BeginGroup();

        ImGui::Separator();

        bool componentEnabled = component->isEnabled();

        if (ImGui::Checkbox("##enabled", &componentEnabled)) {
            component->enabled(componentEnabled);
        }

        ImGui::SameLine();

        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", component->typeName().c_str());

        ImGui::SameLine();

        float avail = ImGui::GetContentRegionAvail().x;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - 20);

        if (isTransform) {
            ImGui::BeginDisabled();
            ImGui::Button("X");
            ImGui::EndDisabled();
        } else {
            if (ImGui::Button("X")) {
                selectedObject->removeComponent(component.get());
                ImGui::PopID();
                ImGui::EndGroup();
                break;
            }
        }
        ImGui::Indent(10.0f);

        _registry.drawInspector(component.get(), context);

        ImGui::Unindent(10.0f);

        ImGui::Spacing();

        ImGui::EndGroup();

        ImGui::PopID();
    }

    ImGui::Spacing();
    ImGui::Separator();
    if (ImGui::Button("+ Add Component", ImVec2(-1, 0))) {
        _showComponentList = !_showComponentList;
    }

    if (_showComponentList) {

        ImGui::BeginChild("ComponentList", ImVec2(0, 140), true);

        for (auto& factory : _factories) {

            bool alreadyHas = false;

            if (factory.name == "Transform" && selectedObject->getComponent<dzemikk::Transform>()) {
                alreadyHas = true;
            }

            if (factory.name == "MeshRenderer" &&
                selectedObject->getComponent<dzemikk::MeshRenderer>()) {
                alreadyHas = true;
            }

            if (alreadyHas) {
                ImGui::BeginDisabled();
                ImGui::Selectable(factory.name.c_str());
                ImGui::EndDisabled();
            } else {
                if (ImGui::Selectable(factory.name.c_str())) {
                    factory.create(selectedObject);
                    _showComponentList = false;
                }
            }
        }

        ImGui::EndChild();
    }

    ImGui::End();
}