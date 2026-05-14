#include "inspectorPanel.h"

#include "ecs/components/transform.h"

#include "inspectors/transformInspector.h"
#include "inspectors/meshRendererInspector.h"
#include "inspectors/directionalLightInspector.h"
#include "inspectors/pointLightInspector.h"

#include <imgui.h>
#include "ecs/components/meshRenderer.h"
#include "ecs/components/light/directionalLight.h"
#include "ecs/components/light/pointLight.h"

editor::InspectorPanel::InspectorPanel() {

    registerInspector<dzemikk::Transform, TransformInspector>("Transform");
    registerInspector<dzemikk::MeshRenderer, MeshRendererInspector>("MeshRenderer");
    registerInspector<dzemikk::DirectionalLight, DirectionalLightInspector>("DirectionalLight");
    registerInspector<dzemikk::PointLight, PointLightInspector>("PointLight");


    _factories = {
        {"Transform", [](auto* go) { return go->getComponent<dzemikk::Transform>() != nullptr; },
         [](auto* go) { go->addComponent<dzemikk::Transform>(); }},

        {"MeshRenderer",
         [](auto* go) { return go->getComponent<dzemikk::MeshRenderer>() != nullptr; },
         [](auto* go) {
             auto r = go->addComponent<dzemikk::MeshRenderer>();
             r->setTransform(go->getComponent<dzemikk::Transform>());
         }},

        {"DirectionalLight",
         [](auto* go) { return go->getComponent<dzemikk::DirectionalLight>() != nullptr; },
         [](auto* go) { go->addComponent<dzemikk::DirectionalLight>(); }},

        {"PointLight", [](auto* go) { return go->getComponent<dzemikk::PointLight>() != nullptr; },
         [](auto* go) { go->addComponent<dzemikk::PointLight>(); }}};

}

void editor::InspectorPanel::draw(dzemikk::GameObject* obj, const InspectorContext& ctx) {
    ImGui::Begin("Inspector");

    if (!obj) {
        ImGui::Text("No object selected");
        ImGui::End();
        return;
    }

    drawHeader(obj);
    drawComponents(obj, ctx);
    drawAddComponent(obj);

    ImGui::End();
}

void editor::InspectorPanel::drawHeader(dzemikk::GameObject* obj) {
    char nameBuffer[256];
    strncpy(nameBuffer, obj->getName().c_str(), sizeof(nameBuffer));
    nameBuffer[255] = '\0';

    if (ImGui::InputText("##name", nameBuffer, sizeof(nameBuffer))) {
        obj->setName(nameBuffer);
    }

    bool enabled = obj->isEnabled();
    if (ImGui::Checkbox("Enabled", &enabled)) {
        obj->enabled(enabled);
    }

    ImGui::Separator();
}

void editor::InspectorPanel::drawComponents(dzemikk::GameObject* obj,
                                            const editor::InspectorContext& ctx) {
    ImGui::TextUnformatted("Components");
    ImGui::Spacing();

    auto& components = obj->getAllComponents();

    for (size_t i = 0; i < components.size(); ++i) {
        auto& component = components[i];

        ImGui::PushID(component.get());

        ImGui::Separator();

        bool isTransform = dynamic_cast<dzemikk::Transform*>(component.get()) != nullptr;

        if (ImGui::BeginTable("component_row", 3,
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody)) {

            ImGui::TableSetupColumn("enabled", ImGuiTableColumnFlags_WidthFixed, 20.0f);
            ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("remove", ImGuiTableColumnFlags_WidthFixed, 30.0f);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            bool enabled = component->isEnabled();
            if (ImGui::Checkbox("##enabled", &enabled)) {
                component->enabled(enabled);
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(component->typeName().c_str());

            ImGui::TableSetColumnIndex(2);

            bool removed = false;

            if (isTransform) {
                ImGui::BeginDisabled();
                ImGui::Button("X");
                ImGui::EndDisabled();
            } else {
                if (ImGui::Button("X")) {
                    obj->removeComponent(component.get());
                    removed = true;
                }
            }

            ImGui::EndTable();

            if (removed) {
                ImGui::PopID();
                break;
            }
        }

        ImGui::Indent();
        _registry.drawInspector(component.get(), ctx);
        ImGui::Unindent();

        ImGui::PopID();
    }
}

void editor::InspectorPanel::drawAddComponent(dzemikk::GameObject* obj) {
    if (ImGui::Button("+ Add Component", ImVec2(-1, 0))) {
        _showComponentList = !_showComponentList;
    }

    if (!_showComponentList)
        return;

    ImGui::BeginChild("ComponentList", ImVec2(0, 140), true);

    for (auto& factory : _factories) {
        bool exists = factory.has(obj);

        if (exists) {
            ImGui::BeginDisabled();
            ImGui::Selectable(factory.name.c_str());
            ImGui::EndDisabled();
        } else {
            if (ImGui::Selectable(factory.name.c_str())) {
                factory.create(obj);
                _showComponentList = false;
            }
        }
    }

    ImGui::EndChild();
}