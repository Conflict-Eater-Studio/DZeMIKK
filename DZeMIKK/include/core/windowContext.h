#ifndef DZEMIKK_WINDOWCONTEXT_H
#define DZEMIKK_WINDOWCONTEXT_H

#include <glm/glm.hpp>

namespace dzemikk {
class WindowContext {
  public:
    WindowContext(const WindowContext&) = delete;
    WindowContext& operator=(const WindowContext&) = delete;
    WindowContext(WindowContext&&) = delete;
    WindowContext& operator=(WindowContext&&) = delete;

    static WindowContext& get() {
        static WindowContext instance;
        return instance;
    }

    void setWindowSize(glm::ivec2 size) {
        _windowSize = size;
    }

    [[nodiscard]] glm::vec2 getWindowSize() const {
        return _windowSize;
    }

  private:
    WindowContext() = default;
    ~WindowContext() = default;

    glm::ivec2 _windowSize{1920, 1080};
};
} // namespace dzemikk

#endif // DZEMIKK_WINDOWCONTEXT_H
