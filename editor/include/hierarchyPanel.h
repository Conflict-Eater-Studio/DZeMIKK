#pragma once

#include "ecs/gameobject.h"
#include "ecs/scene.h"

namespace editor {
class Editor;
class HierarchyPanel {
  public:
    void setEditor(Editor* editor);
    void draw(dzemikk::Scene* scene, dzemikk::GameObject*& selectedObject);

  private:
    void drawNode(dzemikk::GameObject* gameObject, dzemikk::GameObject*& selectedObject);

    Editor* _editor = nullptr;
};

} // namespace editor