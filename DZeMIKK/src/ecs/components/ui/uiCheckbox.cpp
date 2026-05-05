#include "ecs/components/ui/uiCheckbox.h"

#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/rectTransform.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <glm/geometric.hpp>
#include <glm/matrix.hpp>

namespace dzemikk {
namespace {
constexpr float kMinDeterminant = 1e-6F;
}

void UICheckbox::processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                                bool releasedThisFrame, double scrollDelta) {
    (void)scrollDelta;

    setPointerDown(isDown);
    setPointerInside(_owner != nullptr && _owner->rectTransform() != nullptr &&
                     _owner->rectTransform()->containsPoint(point));
    updateHoverState();
    processStandardPressRelease(pressedThisFrame, releasedThisFrame);
    applyVisualState();
}

void UICheckbox::onClick() {
    _value = !_value;
    if (_checkmarkSpriteRenderer != nullptr) {
        _checkmarkSpriteRenderer->enabled(_value);
    }

    emit(UIEventType::Click, _value);
}

void UICheckbox::setBackgroundSpriteRenderer(ImageRenderer* spriteRenderer) {
    _backgroundSpriteRenderer = spriteRenderer;
    applyVisualState();
}

void UICheckbox::setCheckmarkSpriteRenderer(ImageRenderer* spriteRenderer) {
    _checkmarkSpriteRenderer = spriteRenderer;
    if (_checkmarkSpriteRenderer != nullptr) {
        _checkmarkSpriteRenderer->enabled(_value);
    }

    applyVisualState();
}

void UICheckbox::setStyle(const Style& style) {
    _style = style;
    applyVisualState();
}

UICheckbox::Style UICheckbox::getStyle() const {
    return _style;
}

void UICheckbox::applyVisualState() {
    if (_backgroundSpriteRenderer == nullptr) {
        return;
    }

    if (_checkmarkSpriteRenderer != nullptr) {
        _checkmarkSpriteRenderer->setColor(_style.checkmarkColor);
    }

    if (pressedInside() && pointerInside() && pointerDown()) {
        _backgroundSpriteRenderer->setColor(_style.pressedColor);
    } else if (isHovered()) {
        _backgroundSpriteRenderer->setColor(_style.hoverColor);
    } else {
        _backgroundSpriteRenderer->setColor(_style.normalColor);
    }
}
} // namespace dzemikk
