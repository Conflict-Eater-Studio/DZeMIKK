#ifndef EDITOR_INSPECTOR_PANEL_INSPECTOR_H
#define EDITOR_INSPECTOR_PANEL_INSPECTOR_H

#include "ecs/gameobject.h"
#include "inspectors/inspectorRegistry.h"

#include <functional>
#include <string>
#include <vector>

namespace editor {

struct InspectorContext;

/**
 * @brief Factory describing how a component can be created and queried on a GameObject.
 */
struct ComponentFactory {
    std::string name;

    /// Returns true if the GameObject already has this component type.
    std::function<bool(dzemikk::GameObject*)> has;

    /// Creates and attaches the component to the GameObject.
    std::function<void(dzemikk::GameObject*)> create;
};

/**
 * @brief Main inspector panel responsible for displaying and editing GameObjects.
 *
 * Uses registered inspectors to render component UIs dynamically.
 */
class InspectorPanel {
  public:
#pragma region Lifecycle

    /**
     * @brief Constructs the inspector panel and initializes default state.
     */
    InspectorPanel();

#pragma endregion

#pragma region Drawing

    /**
     * @brief Renders the inspector UI for the selected GameObject.
     *
     * @param selectedObject Currently selected GameObject.
     * @param context Shared inspector context.
     */
    void draw(dzemikk::GameObject* selectedObject, const InspectorContext& context);

#pragma endregion

  private:
#pragma region Registry / Factories

    InspectorRegistry _registry;
    std::vector<ComponentFactory> _factories;

#pragma endregion

#pragma region State

    bool _showComponentList = false;

#pragma endregion

#pragma region Helpers

    /**
     * @brief Registers a typed component inspector into the registry.
     *
     * @tparam T Component type.
     * @tparam Inspector Inspector class implementing static draw().
     * @param name String identifier of the component type.
     */
    template <typename T, typename Inspector> void registerInspector(const std::string& name) {
        _registry.registerInspector(name, [](dzemikk::Component* c, const InspectorContext& ctx) {
            if (auto* obj = dynamic_cast<T*>(c)) {
                Inspector::draw(obj, ctx);
            }
        });
    }

    /**
     * @brief Draws the header section of the inspector (name, enabled state, etc.).
     */
    void drawHeader(dzemikk::GameObject* obj);

    /**
     * @brief Draws all components attached to the GameObject.
     */
    void drawComponents(dzemikk::GameObject* obj, const InspectorContext& ctx);

    /**
     * @brief Draws the "Add Component" UI section.
     */
    void drawAddComponent(dzemikk::GameObject* obj);

#pragma endregion
};

} // namespace editor

#endif