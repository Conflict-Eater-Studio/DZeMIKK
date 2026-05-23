#ifndef EDITOR_ASSET_HIERARCHY_PANEL_INSPECTOR_H
#define EDITOR_ASSET_HIERARCHY_PANEL_INSPECTOR_H

#include "ecs/gameobject.h"
#include "ecs/scene.h"

namespace editor {

class Editor;

/**
 * @brief Editor panel responsible for displaying the scene hierarchy.
 *
 * Allows browsing and selecting GameObjects within the active Scene.
 */
class HierarchyPanel {
  public:
#pragma region Editor Binding

    /**
     * @brief Sets the owning editor instance.
     *
     * @param editor Pointer to the main Editor.
     */
    void setEditor(Editor* editor);

#pragma endregion

#pragma region Drawing

    /**
     * @brief Renders the hierarchy panel UI.
     *
     * @param scene Active scene being displayed.
     * @param selectedObject Currently selected GameObject reference.
     */
    void draw(dzemikk::Scene* scene, dzemikk::GameObject*& selectedObject);

#pragma endregion

  private:
#pragma region Helpers

    /**
     * @brief Recursively draws a GameObject node and its children.
     *
     * @param gameObject Current GameObject node.
     * @param selectedObject Currently selected GameObject reference.
     */
    void drawNode(dzemikk::GameObject* gameObject, dzemikk::GameObject*& selectedObject);

#pragma endregion

  private:
    Editor* _editor = nullptr; 
};

} // namespace editor

#endif