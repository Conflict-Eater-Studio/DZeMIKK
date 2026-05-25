#ifndef EDITOR_SCENEPANEL_H
#define EDITOR_SCENEPANEL_H

#include <glm/vec2.hpp>

namespace dzemikk {
class Renderer;
}

namespace editor {

/**
 * @brief Editor panel responsible for rendering the game scene viewport.
 *
 * Displays the rendered output from the engine Renderer and tracks
 * the viewport size for editor interactions.
 */
class ScenePanel {
  public:
#pragma region Lifecycle

    ScenePanel() = default;
    ~ScenePanel() = default;

    ScenePanel(const ScenePanel&) = delete;
    ScenePanel& operator=(const ScenePanel&) = delete;

    ScenePanel(ScenePanel&&) = delete;
    ScenePanel& operator=(ScenePanel&&) = delete;

#pragma endregion

#pragma region Drawing

    /**
     * @brief Renders the scene viewport inside the editor.
     *
     * @param renderer Pointer to the engine Renderer.
     */
    void draw(dzemikk::Renderer* renderer);

#pragma endregion

#pragma region Getters

    /**
     * @brief Returns the current size of the viewport.
     *
     * @return glm::vec2 Width and height of the viewport.
     */
    [[nodiscard]] glm::vec2 getViewportSize() const {
        return _viewportSize;
    }

#pragma endregion

  private:
    glm::vec2 _viewportSize = {0.0F, 0.0F};
};

} // namespace editor

#endif