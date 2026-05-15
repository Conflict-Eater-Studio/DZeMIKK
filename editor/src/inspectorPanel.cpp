#include "inspectorPanel.h"

#include "ecs/components/transform.h"
#include "ecs/components/ui/rectTransform.h"

#include "inspectors/transformInspector.h"
#include "inspectors/meshRendererInspector.h"
#include "inspectors/directionalLightInspector.h"
#include "inspectors/pointLightInspector.h"
#include "inspectors/spotLightInspector.h"
#include "inspectors/canvasInspector.h"
#include "inspectors/rectTransformInspector.h"
#include "inspectors/imageRendererInspector.h"
#include "inspectors/gridLayoutInspector.h"
#include "inspectors/horizontalLayoutInspector.h"
#include "inspectors/verticalLayoutInspector.h"
#include "inspectors/cameraInspector.h"
#include "inspectors/colliderInspector.h"
#include "inspectors/spriteRendererInspector.h"
#include "inspectors/uITextRendererInspector.h"
#include "inspectors/textRendererInspector.h"
#include "inspectors/skinnedMeshRendererInspector.h"

#include <imgui.h>
#include "ecs/components/meshRenderer.h"
#include "ecs/components/skinnedMeshRenderer.h"
#include "ecs/components/camera.h"
#include "ecs/components/collider.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/textRenderer.h"
#include "ecs/components/skinnedMeshRenderer.h"
#include "ecs/components/light/directionalLight.h"
#include "ecs/components/light/pointLight.h"
#include "ecs/components/light/spotLight.h"
#include "ecs/components/ui/canvas.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/gridLayout.h"
#include "ecs/components/ui/horizontalLayout.h"
#include "ecs/components/ui/verticalLayout.h"
#include "ecs/components/ui/uiTextRenderer.h"

editor::InspectorPanel::InspectorPanel() {

    registerInspector<dzemikk::Transform, TransformInspector>("Transform");
    registerInspector<dzemikk::MeshRenderer, MeshRendererInspector>("MeshRenderer");
    registerInspector<dzemikk::DirectionalLight, DirectionalLightInspector>("DirectionalLight");
    registerInspector<dzemikk::PointLight, PointLightInspector>("PointLight");
    registerInspector<dzemikk::SpotLight, SpotLightInspector>("SpotLight");
    registerInspector<dzemikk::Canvas, CanvasInspector>("Canvas");
    registerInspector<dzemikk::RectTransform, RectTransformInspector>("RectTransform");
    registerInspector<dzemikk::ImageRenderer, ImageRendererInspector>("ImageRenderer");
    registerInspector<dzemikk::GridLayout, GridLayoutInspector>("GridLayout");
    registerInspector<dzemikk::HorizontalLayout, HorizontalLayoutInspector>("HorizontalLayout");
    registerInspector<dzemikk::VerticalLayout, VerticalLayoutInspector>("VerticalLayout");
    registerInspector<dzemikk::Camera, CameraInspector>("Camera");
    registerInspector<dzemikk::Collider, ColliderInspector>("Collider");
    registerInspector<dzemikk::SpriteRenderer, SpriteRendererInspector>("SpriteRenderer");
    registerInspector<dzemikk::UITextRenderer, UITextRendererInspector>("UITextRenderer");
    registerInspector<dzemikk::TextRenderer, TextRendererInspector>("TextRenderer");
    registerInspector<dzemikk::SkinnedMeshRenderer, SkinnedMeshRendererInspector>("SkinnedMeshRenderer");

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
         [](auto* go) { go->addComponent<dzemikk::PointLight>(); }},
    
        {"SpotLight", [](auto* go) { return go->getComponent<dzemikk::SpotLight>() != nullptr; },
         [](auto* go) { go->addComponent<dzemikk::SpotLight>(); }},

        {"Canvas", [](auto* go) { return go->getComponent<dzemikk::Canvas>() != nullptr; },
         [](auto* go) { go->addComponent<dzemikk::Canvas>(); }},

        {"RectTransform",
         [](auto* go) { return go->getComponent<dzemikk::RectTransform>() != nullptr; },
         [](auto* go) { go->addComponent<dzemikk::RectTransform>(); }},

        {"ImageRenderer",
         [](auto* go) { return go->getComponent<dzemikk::ImageRenderer>() != nullptr; },
         [](auto* go) { 
            auto i = go->addComponent<dzemikk::ImageRenderer>(); 
            i->setRectTransform(go->getComponent<dzemikk::RectTransform>());
        }},

        {"GridLayout", [](auto* go) { return go->getComponent<dzemikk::GridLayout>() != nullptr; },
         [](auto* go) { go->addComponent<dzemikk::GridLayout>(); }},

        {"HorizontalLayout",
         [](auto* go) { return go->getComponent<dzemikk::HorizontalLayout>() != nullptr; },
         [](auto* go) { go->addComponent<dzemikk::HorizontalLayout>(); }},

        {"VerticalLayout",
         [](auto* go) { return go->getComponent<dzemikk::VerticalLayout>() != nullptr; },
         [](auto* go) { go->addComponent<dzemikk::VerticalLayout>(); }},

        {"Camera", [](auto* go) { return go->getComponent<dzemikk::Camera>() != nullptr; },
         [](auto* go) { go->addComponent<dzemikk::Camera>(); }},

        {"Collider", [](auto* go) { return go->getComponent<dzemikk::Collider>() != nullptr; },
         [](auto* go) { 
            auto collider = go->addComponent<dzemikk::Collider>();
             if (go->getComponent<dzemikk::MeshRenderer>()) {
                 auto r =  go->getComponent<dzemikk::MeshRenderer>();
                 collider->setModel(r->getModel());
             } else if (go->getComponent<dzemikk::SkinnedMeshRenderer>()) {
                 auto r = go->getComponent<dzemikk::SkinnedMeshRenderer>();
                 collider->setModel(r->getModel());
             }

             collider->setTransform(go->transform());
        }},

        {"SpriteRenderer",
         [](auto* go) { return go->getComponent<dzemikk::SpriteRenderer>() != nullptr; },
         [](auto* go) { 
            auto r = go->addComponent<dzemikk::SpriteRenderer>(); 
            r->setTransform(go->getComponent<dzemikk::Transform>());
        }},

        {"UITextRenderer", [](auto* go) { return go->getComponent<dzemikk::UITextRenderer>() != nullptr; },
         [](auto* go) { go->addComponent<dzemikk::UITextRenderer>(); }},

        {"TextRenderer",
         [](auto* go) { return go->getComponent<dzemikk::TextRenderer>() != nullptr; },
         [](auto* go) { go->addComponent<dzemikk::TextRenderer>(); }},

        {"SkinnedMeshRenderer",
         [](auto* go) { return go->getComponent<dzemikk::SkinnedMeshRenderer>() != nullptr; },
         [](auto* go) {
             auto r = go->addComponent<dzemikk::SkinnedMeshRenderer>();
             r->setTransform(go->getComponent<dzemikk::Transform>());
         }},
    };

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
        bool isRectTransform = dynamic_cast<dzemikk::RectTransform*>(component.get()) != nullptr;

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

            if (isTransform || isRectTransform) {
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

        auto* imageRenderer = dynamic_cast<dzemikk::ImageRenderer*>(component.get());

        if (imageRenderer && !imageRenderer->getMesh()) {

            auto quad = ctx.assetManager->getPrimitiveMesh(
                dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);

            imageRenderer->setMesh(quad);
        }

        auto* spriteRenderer = dynamic_cast<dzemikk::SpriteRenderer*>(component.get());

        if (spriteRenderer && !spriteRenderer->getMesh()) {

            auto quad = ctx.assetManager->getPrimitive(
                dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);

            spriteRenderer->setMesh(quad);
        }

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