#pragma once

#include "ecs/gameobject.h"

#include "inspectors/inspectorRegistry.h"

namespace editor {
struct ComponentFactory {
    std::string name;
    std::function<void(dzemikk::GameObject*)> create;
};

class InspectorPanel {
  public:
    InspectorPanel();
    void draw(dzemikk::GameObject* selectedObject, const InspectorContext& context);

  private:
    InspectorRegistry _registry;
    std::vector<ComponentFactory> _factories;

    bool _showComponentList = false;
};

} // namespace editor