#pragma once

#include "ecs/gameobject.h"

namespace editor {

class InspectorPanel {
  public:
    void draw(dzemikk::GameObject* selectedObject);
};

} // namespace editor