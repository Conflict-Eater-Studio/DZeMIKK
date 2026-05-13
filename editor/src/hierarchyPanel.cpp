#include "hierarchyPanel.h"

#include <imgui.h>

void editor::HierarchyPanel::draw(dzemikk::Scene* scene, dzemikk::GameObject*& selectedObject) {
    ImGui::Begin("Hierarchy");

    if (!scene) {
        ImGui::Text("No scene");
        ImGui::End();
        return;
    }

    const auto& objects = scene->getObjects();

    for (const auto& go : objects) {
        if (go->getParent() == nullptr) {
            drawNode(go.get(), selectedObject);
        }
    }

    ImGui::End();
}

void editor::HierarchyPanel::drawNode(dzemikk::GameObject* gameObject,
                                      dzemikk::GameObject*& selectedObject) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (selectedObject == gameObject) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (gameObject->getChildren().empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool opened = ImGui::TreeNodeEx((void*)gameObject, flags, "%s", gameObject->getName().c_str());

    if (ImGui::IsItemClicked()) {
        selectedObject = gameObject;
    }

    if (opened) {

        for (auto* child : gameObject->getChildren()) {
            drawNode(child, selectedObject);
        }

        ImGui::TreePop();
    }
}
