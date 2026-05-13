#pragma once

#include "ecs/gameobject.h"

#include "inspectors/inspectorRegistry.h"

namespace editor {

class InspectorPanel {
  public:
    InspectorPanel();
    void draw(dzemikk::GameObject* selectedObject, const InspectorContext& context);

  private:
    InspectorRegistry _registry;
};

} // namespace editor