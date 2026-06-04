#include "inspectorPanel.h"

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
#include "inspectors/uIButtonInspector.h"
#include "inspectors/uICheckboxInspector.h"
#include "inspectors/uIDropdownInspector.h"
#include "inspectors/uISliderInspector.h"
#include "inspectors/animatorInspector.h"

#include <imgui.h>

#include <ecs/components/transform.h>
#include <ecs/components/ui/rectTransform.h>
#include <ecs/components/meshRenderer.h>
#include <ecs/components/skinnedMeshRenderer.h>
#include <ecs/components/camera.h>
#include <ecs/components/collider.h>
#include <ecs/components/spriteRenderer.h>
#include <ecs/components/textRenderer.h>
#include <ecs/components/light/directionalLight.h>
#include <ecs/components/light/pointLight.h>
#include <ecs/components/light/spotLight.h>
#include <ecs/components/ui/canvas.h>
#include <ecs/components/ui/imageRenderer.h>
#include <ecs/components/ui/gridLayout.h>
#include <ecs/components/ui/horizontalLayout.h>
#include <ecs/components/ui/verticalLayout.h>
#include <ecs/components/ui/uiTextRenderer.h>
#include <ecs/components/ui/uiButton.h>
#include <ecs/components/ui/uiBuilder.h>
#include <ecs/components/ui/uiCheckbox.h>
#include <ecs/components/ui/uiDropdown.h>
#include <ecs/components/ui/uiSlider.h>
#include <ecs/components/animator.h>

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
    registerInspector<dzemikk::UIButton, UIButtonInspector>("UIButton");
    registerInspector<dzemikk::Animator, AnimatorInspector>("Animator");
    registerInspector<dzemikk::UICheckbox, UICheckboxInspector>("UICheckbox");
    registerInspector<dzemikk::UIDropdown, UIDropdownInspector>("UIDropdown");
    registerInspector<dzemikk::UISlider, UISliderInspector>("UISlider");

    _factories = std::vector<ComponentFactory>{
        {.name = "Transform",
         .has = [](auto* go) { return go->getComponent<dzemikk::Transform>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::Transform>(); }},

        {.name = "MeshRenderer",
         .has = [](auto* go) { return go->getComponent<dzemikk::MeshRenderer>() != nullptr; },
         .create =
             [](auto* go) {
                 auto r = go->addComponent<dzemikk::MeshRenderer>();
                 r->setTransform(go->getComponent<dzemikk::Transform>());
             }},

        {.name = "DirectionalLight",
         .has = [](auto* go) { return go->getComponent<dzemikk::DirectionalLight>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::DirectionalLight>(); }},

        {.name = "PointLight",
         .has = [](auto* go) { return go->getComponent<dzemikk::PointLight>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::PointLight>(); }},

        {.name = "SpotLight",
         .has = [](auto* go) { return go->getComponent<dzemikk::SpotLight>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::SpotLight>(); }},

        {.name = "Canvas",
         .has = [](auto* go) { return go->getComponent<dzemikk::Canvas>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::Canvas>(); }},

        {.name = "RectTransform",
         .has = [](auto* go) { return go->getComponent<dzemikk::RectTransform>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::RectTransform>(); }},

        {.name = "ImageRenderer",
         .has = [](auto* go) { return go->getComponent<dzemikk::ImageRenderer>() != nullptr; },
         .create =
             [](auto* go) {
                 auto i = go->addComponent<dzemikk::ImageRenderer>();
                 i->setRectTransform(go->getComponent<dzemikk::RectTransform>());
             }},

        {.name = "GridLayout",
         .has = [](auto* go) { return go->getComponent<dzemikk::GridLayout>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::GridLayout>(); }},

        {.name = "HorizontalLayout",
         .has = [](auto* go) { return go->getComponent<dzemikk::HorizontalLayout>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::HorizontalLayout>(); }},

        {.name = "VerticalLayout",
         .has = [](auto* go) { return go->getComponent<dzemikk::VerticalLayout>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::VerticalLayout>(); }},

        {.name = "Camera",
         .has = [](auto* go) { return go->getComponent<dzemikk::Camera>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::Camera>(); }},

        {.name = "Collider",
         .has = [](auto* go) { return go->getComponent<dzemikk::Collider>() != nullptr; },
         .create =
             [](auto* go) {
                 auto collider = go->addComponent<dzemikk::Collider>();

                 if (auto* r = go->getComponent<dzemikk::MeshRenderer>()) {
                     collider->setModel(r->getModel());
                 } else if (auto* r = go->getComponent<dzemikk::SkinnedMeshRenderer>()) {
                     collider->setModel(r->getModel());
                 }

                 collider->setTransform(go->transform());
             }},

        {.name = "SpriteRenderer",
         .has = [](auto* go) { return go->getComponent<dzemikk::SpriteRenderer>() != nullptr; },
         .create =
             [](auto* go) {
                 auto r = go->addComponent<dzemikk::SpriteRenderer>();
                 r->setTransform(go->getComponent<dzemikk::Transform>());
             }},

        {.name = "UITextRenderer",
         .has = [](auto* go) { return go->getComponent<dzemikk::UITextRenderer>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::UITextRenderer>(); }},

        {.name = "TextRenderer",
         .has = [](auto* go) { return go->getComponent<dzemikk::TextRenderer>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::TextRenderer>(); }},

        {.name = "SkinnedMeshRenderer",
         .has =
             [](auto* go) { return go->getComponent<dzemikk::SkinnedMeshRenderer>() != nullptr; },
         .create =
             [](auto* go) {
                 auto r = go->addComponent<dzemikk::SkinnedMeshRenderer>();
                 r->setTransform(go->getComponent<dzemikk::Transform>());
             }},

        {.name = "Animator",
         .has = [](auto* go) { return go->getComponent<dzemikk::Animator>() != nullptr; },
         .create = [](auto* go) { go->addComponent<dzemikk::Animator>(); }},
    };

}

void editor::InspectorPanel::draw(dzemikk::GameObject* selectedObject,
                                  const InspectorContext& context) {
    ImGui::Begin("Inspector");

    if (!selectedObject) {
        ImGui::TextUnformatted("No object selected");
        ImGui::End();
        return;
    }

    drawHeader(selectedObject);
    drawComponents(selectedObject, context);
    drawAddComponent(selectedObject);

    ImGui::End();
}

void editor::InspectorPanel::drawHeader(dzemikk::GameObject* obj) {
    std::array<char, 256> nameBuffer{};

    std::strncpy(nameBuffer.data(), obj->getName().c_str(), nameBuffer.size() - 1);

    if (ImGui::InputText("##name", nameBuffer.data(), nameBuffer.size())) {
        obj->setName(nameBuffer.data());
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

    const auto& components = obj->getAllComponents();

    for (const auto& component : components) {
        ImGui::PushID(component.get());

        ImGui::Separator();

        bool isTransform = dynamic_cast<dzemikk::Transform*>(component.get()) != nullptr;
        bool isRectTransform = dynamic_cast<dzemikk::RectTransform*>(component.get()) != nullptr;

        if (ImGui::BeginTable("component_row", 3,
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody)) {

            ImGui::TableSetupColumn("enabled", ImGuiTableColumnFlags_WidthFixed, 20.0F);
            ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("remove", ImGuiTableColumnFlags_WidthFixed, 30.0F);

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

            auto *quad = ctx.assetManager->getPrimitive(
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

    if (_showComponentList) {

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

    ImGui::Dummy(ImVec2(0.0F, 300.0F));
}