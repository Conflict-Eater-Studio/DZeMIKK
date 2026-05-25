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

    /**
     * @brief Renders drag & drop source for a GameObject in the hierarchy.
     *
     * Allows the GameObject to be dragged from the hierarchy tree.
     * Sets a drag payload used for reparenting or external operations.
     *
     * @param gameObject GameObject being dragged.
     */
    static void drawDragSource(dzemikk::GameObject* gameObject);

    /**
     * @brief Handles drag & drop target interactions for a GameObject node.
     *
     * Accepts dragged GameObjects or assets (e.g. prefabs) and performs
     * appropriate actions such as reparenting or instantiating prefabs.
     *
     * @param gameObject Target GameObject receiving the drop.
     */
    void drawDragTarget(dzemikk::GameObject* gameObject);

    /**
     * @brief Renders context menu for a hierarchy node.
     *
     * Provides GameObject-specific actions such as:
     * - Creating children
     * - Adding UI components
     * - Deleting object
     *
     * @param gameObject GameObject for which the context menu is shown.
     */
    void drawContextMenu(dzemikk::GameObject* gameObject);

#pragma endregion

    Editor* _editor = nullptr; 
};

} // namespace editor

#endif