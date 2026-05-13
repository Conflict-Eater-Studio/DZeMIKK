#pragma once

#include "core/engine.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <memory>

namespace editor {

class Editor {
  public:
    explicit Editor(dzemikk::Engine* engine);
    ~Editor() = default;

    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;

    void start();

  private:
    void setupEditor();
    void registerCallbacks();

    void renderDockspace();

    void drawHierarchyPanel();
    void drawInspectorPanel();
    void drawScenePanel();

    void drawGameObjectNode(dzemikk::GameObject* gameObject);

  private:
    dzemikk::Engine* _engine = nullptr;

    dzemikk::Scene* _activeScene = nullptr;
    dzemikk::GameObject* _selectedObject = nullptr;

    bool _showHierarchy = true;
    bool _showInspector = true;
    bool _showScene = true;

    bool _editorInitialized = false;
};

} // namespace editor