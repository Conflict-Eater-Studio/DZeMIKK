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

    ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 30.0f));

    if (ImGui::BeginDragDropTarget()) {

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT")) {

            auto* dragged = *static_cast<dzemikk::GameObject**>(payload->Data);

            if (dragged && _editor) {
                _editor->reparentObject(dragged, nullptr);
            }
        }

        ImGui::EndDragDropTarget();
    }

    ImVec2 size = ImGui::GetContentRegionAvail();

    if (ImGui::BeginDragDropTarget()) {

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PREFAB")) {

            std::string path = (const char*)payload->Data;

            if (_editor) {
                _editor->instantiatePrefab(path, nullptr); 
            }
        }

        ImGui::EndDragDropTarget();
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

    if (ImGui::BeginDragDropSource()) {

        ImGui::SetDragDropPayload("GAMEOBJECT", &gameObject, sizeof(dzemikk::GameObject*));

        ImGui::Text("%s", gameObject->getName().c_str());

        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_GAMEOBJECT")) {

            auto* dragged = *static_cast<dzemikk::GameObject**>(payload->Data);

            if (dragged && dragged != gameObject && _editor) {

                _editor->reparentObject(dragged, gameObject);
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginDragDropTarget()) {

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PREFAB")) {

            const char* path = (const char*)payload->Data;

            if (_editor) {
                _editor->instantiatePrefab(path, gameObject); 
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginDragDropSource()) {

        dzemikk::GameObject* ptr = gameObject;

        ImGui::SetDragDropPayload("HIERARCHY_GAMEOBJECT", &ptr, sizeof(dzemikk::GameObject*));

        ImGui::Text("%s", gameObject->getName().c_str());

        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginPopupContextItem("NodeContext")) {
        if (_editor) {
            if (ImGui::MenuItem("Create Child")) {
                _editor->createEmptyObject("Empty", gameObject);
            }

            if (ImGui::MenuItem("CreateButton")) {
                _editor->createUIButton(gameObject);
            }

            if (ImGui::MenuItem("Delete")) {
                _editor->deleteObject(gameObject);
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