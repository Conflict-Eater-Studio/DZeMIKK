#ifndef EDITOR_H
#define EDITOR_H

#include <core/engine.h>
#include <ecs/gameobject.h>
#include <ecs/scene.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#if DZEMIKK_DEV_TOOLS
#include <imgui.h>
#include <imgui_internal.h>
#endif

namespace editor {

class HierarchyPanel;
class InspectorPanel;
class AssetManagerPanel;
class ScenePanel;

/**
 * @brief Main editor controller managing all editor panels and scene editing state.
 *
 * Responsible for coordinating UI panels, scene editing operations,
 * object creation/destruction, and prefab instantiation.
 */
class Editor {
  public:
#pragma region Lifecycle

    /**
     * @brief Constructs the editor instance.
     *
     * @param engine Pointer to the engine instance.
     */
    explicit Editor(dzemikk::Engine* engine);

    /**
     * @brief Destructor.
     */
    ~Editor();

    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;

    Editor(Editor&&) noexcept = default;
    Editor& operator=(Editor&&) noexcept = default;

#pragma endregion

#pragma region Runtime

    /**
     * @brief Starts the editor runtime loop.
     */
    void start();

#pragma endregion

#pragma region Scene Operations

    /**
     * @brief Creates an empty GameObject in the scene.
     *
     * @param name Name of the new object.
     * @param parent Optional parent object.
     */
    void createEmptyObject(const std::string& name, dzemikk::GameObject* parent = nullptr);

    /**
     * @brief Creates a UI Button GameObject.
     *
     * @param parent Optional parent object.
     */
    void createUIButton(dzemikk::GameObject* parent);

    /**
     * @brief Creates a UI Checkbox GameObject.
     *
     * @param parent Optional parent object.
     */
    void createUICheckbox(dzemikk::GameObject* parent);

    /**
     * @brief Creates a UI Slider GameObject.
     *
     * @param parent Optional parent object.
     */
    void createUISlider(dzemikk::GameObject* parent);

    /**
     * @brief Creates a UI Dropdown GameObject.
     *
     * @param parent Optional parent object.
     */
    void createUIDropdown(dzemikk::GameObject* parent);

    /**
     * @brief Deletes a GameObject from the scene.
     *
     * @param gameObject Object to delete.
     */
    void deleteObject(dzemikk::GameObject* gameObject);

    /**
     * @brief Changes the parent of a GameObject.
     *
     * @param child Child object.
     * @param parent New parent object.
     */
    void reparentObject(dzemikk::GameObject* child, dzemikk::GameObject* parent);

    /**
     * @brief Instantiates a prefab from file.
     *
     * @param path Path to prefab file.
     * @param parent Optional parent object.
     */
    void instantiatePrefab(const std::string& path, dzemikk::GameObject* parent);

#pragma endregion

  private:
#pragma region Internal Setup / UI

    void setupEditor();
    void renderDockspace();
    void renderBottomBar();

    /**
     * @brief Opens a native save-file dialog.
     *
     * @return Selected file path.
     */
    std::string openSaveFileDialog();

#pragma endregion

#pragma region Core References

    dzemikk::Engine* _engine = nullptr;
    dzemikk::Scene* _activeScene = nullptr;
    dzemikk::GameObject* _selectedObject = nullptr;

#pragma endregion

#pragma region UI State

    bool _showHierarchy = true;
    bool _showInspector = true;
    bool _showAssetManager = true;
    bool _showScene = true;

    bool _editorInitialized = false;

    bool _showSaveScenePopup = false;
    char _scenePathBuffer[256] = "scene.json";

#pragma endregion

#pragma region Panels

    std::unique_ptr<HierarchyPanel> _hierarchyPanel;
    std::unique_ptr<InspectorPanel> _inspectorPanel;
    std::unique_ptr<AssetManagerPanel> _assetManagerPanel;
    std::unique_ptr<ScenePanel> _scenePanel;

#pragma endregion

#pragma region Deferred Execution

    std::vector<std::function<void()>> _deferredOps;

#pragma endregion
};

} // namespace editor

#endif