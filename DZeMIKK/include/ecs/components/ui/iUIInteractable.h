#ifndef DZEMIKK_IUIINTERACTABLE_H
#define DZEMIKK_IUIINTERACTABLE_H

#include "ecs/component.h"

#include <glm/vec2.hpp>

namespace dzemikk {
class IUIInteractable : public Component {
  public:
    using Base = Component;

    [[nodiscard]] std::string typeName() const override {
        return "IUIInteractable";
    }

    [[nodiscard]] virtual bool containsPoint(const glm::vec2& point) const = 0;
    virtual void processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                                bool releasedThisFrame) = 0;

    [[nodiscard]] virtual bool isHovered() const = 0;
    [[nodiscard]] virtual bool isPressed() const = 0;

    virtual void onClick() = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
};
} // namespace dzemikk

#endif // DZEMIKK_IUIINTERACTABLE_H