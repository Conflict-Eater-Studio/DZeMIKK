#include "hierarchyPanel.h"
#include "editor.h"

#include <imgui.h>

void editor::HierarchyPanel::setEditor(Editor* editor) {
    _editor = editor;
}

void editor::HierarchyPanel::draw(dzemikk::Scene* scene, dzemikk::GameObject*& selectedObject) {
    ImGui::Begin("Hierarchy");

    if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight)) {

        if (_editor) {
            if (ImGui::MenuItem("Create Empty")) {
                _editor->createEmptyObject("Empty", nullptr);
            }
        }

        ImGui::EndPopup();
    }

    if (!scene) {
        ImGui::Text("No scene");
        ImGui::End();
        return;
    }

    const auto& objects = scene->getObjects();

    for (const auto& go : objects) {
        if (!go) {
            continue;
        }

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

    if (ImGui::BeginPopupContextItem("NodeContext")) {
        if (_editor) {
            if (ImGui::MenuItem("Create Child")) {
                _editor->createEmptyObject("Empty", gameObject);
            }
        }
        ImGui::EndPopup();
    }

    if (opened) {
        for (auto* child : gameObject->getChildren()) {
            drawNode(child, selectedObject);
        }
        ImGui::TreePop();
    }
}