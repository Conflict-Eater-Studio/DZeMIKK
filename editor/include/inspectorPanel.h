#pragma once

#include "ecs/gameobject.h"

#include "inspectors/inspectorRegistry.h"

namespace editor {
struct ComponentFactory {
    std::string name;
    std::function<bool(dzemikk::GameObject*)> has;
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

    template <typename T, typename Inspector> void registerInspector(const std::string& name) {
        _registry.registerInspector(name, [](dzemikk::Component* c, const InspectorContext& ctx) {
            if (auto* obj = dynamic_cast<T*>(c)) {
                Inspector::draw(obj, ctx);
            }
        });
    }

    void drawHeader(dzemikk::GameObject* obj);
    void drawComponents(dzemikk::GameObject* obj, const InspectorContext& ctx);
    void drawAddComponent(dzemikk::GameObject* obj);
};

} // namespace editor