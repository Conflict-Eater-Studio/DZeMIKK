#ifndef EDITOR_SCENEPANEL_H
#define EDITOR_SCENEPANEL_H

#include <glm/vec2.hpp>

namespace dzemikk {
class Renderer;
}

namespace editor {

class ScenePanel {
  public:
    ScenePanel() = default;
    ~ScenePanel() = default;

    void draw(dzemikk::Renderer* renderer);

    [[nodiscard]] glm::vec2 getViewportSize() const {
        return _viewportSize;
    }

  private:
    glm::vec2 _viewportSize = {0.0f, 0.0f};
};

} // namespace editor

#endif