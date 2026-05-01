#include "ecs/components/ui/uiButton.h"

#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/rectTransform.h"
#include "ecs/gameobject.h"

#include <glm/matrix.hpp>

namespace dzemikk {
namespace {
constexpr float kMinDeterminant = 1e-6F;
}

void UIButton::processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                              bool releasedThisFrame) {
    setPointerDown(isDown);
    setPointerInside(_owner != nullptr && _owner->rectTransform() != nullptr &&
                     _owner->rectTransform()->containsPoint(point));
    updateHoverState();
    processStandardPressRelease(pressedThisFrame, releasedThisFrame);
    applyVisualState();
}

void UIButton::setSpriteRenderer(ImageRenderer* spriteRenderer) {
    _spriteRenderer = spriteRenderer;
    applyVisualState();
}

void UIButton::setStyle(const Style& style) {
    _style = style;
    applyVisualState();
}

UIButton::Style UIButton::getStyle() const {
    return _style;
}

void UIButton::applyVisualState() {
    if (_spriteRenderer == nullptr) {
        return;
    }

    if (pressedInside() && pointerInside() && pointerDown()) {
        _spriteRenderer->setColor(_style.pressedColor);
    } else if (isHovered()) {
        _spriteRenderer->setColor(_style.hoverColor);
    } else {
        _spriteRenderer->setColor(_style.normalColor);
    }
}
} // namespace dzemikk
