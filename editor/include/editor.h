#pragma once

#include "core/engine.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <memory>

#if DZEMIKK_DEV_TOOLS

#include <imgui.h>
#include <imgui_internal.h>

#endif

namespace editor {
class HierarchyPanel;
class InspectorPanel;

class Editor {
  public:
    explicit Editor(dzemikk::Engine* engine);
    ~Editor();

    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;

    void start();

    void createEmptyObject(const std::string& name, dzemikk::GameObject* parent = nullptr);

  private:
    void setupEditor();
    void renderDockspace();

  private:
    dzemikk::Engine* _engine = nullptr;

    dzemikk::Scene* _activeScene = nullptr;
    dzemikk::GameObject* _selectedObject = nullptr;

    bool _showHierarchy = true;
    bool _showInspector = true;
    bool _showScene = true;

    bool _editorInitialized = false;

    bool _showSaveScenePopup = false;
    char _scenePathBuffer[256] = "scene.json";

    std::unique_ptr<HierarchyPanel> _hierarchyPanel;
    std::unique_ptr<InspectorPanel> _inspectorPanel;

    std::vector<std::function<void()>> _deferredOps;
};

} // namespace editor