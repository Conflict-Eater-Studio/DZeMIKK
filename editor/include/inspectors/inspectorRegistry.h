#pragma once

#include <assetManager/assetmanager.h>

#include <functional>
#include <string>
#include <unordered_map>

namespace dzemikk {
class Component;
}

namespace editor {

/**
 * @brief Shared context passed to all inspector UI drawers.
 *
 * Provides access to engine systems such as the AssetManager.
 */
struct InspectorContext {
    dzemikk::AssetManager* assetManager = nullptr;
};

/**
 * @brief Central registry for component inspector UI drawers.
 *
 * Maps component type names to their corresponding UI drawing functions,
 * allowing extensible inspector rendering in the editor.
 */
class InspectorRegistry {
  public:
#pragma region Types

    /**
     * @brief Function type used to draw a component inspector UI.
     */
    using DrawFunction = std::function<void(dzemikk::Component*, const InspectorContext&)>;

#pragma endregion

#pragma region Registration

    /**
     * @brief Registers an inspector draw function for a given component type.
     *
     * @param type String identifier of the component type.
     * @param function Function used to draw the inspector UI.
     */
    void registerInspector(const std::string& type, DrawFunction function);

#pragma endregion

#pragma region Drawing

    /**
     * @brief Draws the inspector UI for a given component instance.
     *
     * @param component Component instance to inspect.
     * @param context Shared inspector context.
     */
    void drawInspector(dzemikk::Component* component, const InspectorContext& context);

#pragma endregion

  private:
    std::unordered_map<std::string, DrawFunction> _inspectors;
};

} // namespace editor