#pragma once

#include "ecs/gameobject.h"
#include "ecs/scene.h"

namespace editor {

class HierarchyPanel {
  public:
    void draw(dzemikk::Scene* scene, dzemikk::GameObject*& selectedObject);

  private:
    void drawNode(dzemikk::GameObject* gameObject, dzemikk::GameObject*& selectedObject);
};

} // namespace editor