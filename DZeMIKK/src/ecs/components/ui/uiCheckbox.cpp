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

ImageRenderer* UICheckbox::getBackgroundSpriteRenderer() const {
    if (_backgroundSpriteRenderer == nullptr) {
        _backgroundSpriteRenderer = getOwner()->getComponent<ImageRenderer>();
    }
    return _backgroundSpriteRenderer;
}

ImageRenderer* UICheckbox::getCheckmarkSpriteRenderer() const {
    if (_checkmarkSpriteRenderer == nullptr) {
        const auto& children = getOwner()->getChildren();
        if (!children.empty()) {
            _checkmarkSpriteRenderer = children[0]->getComponent<ImageRenderer>();
        }
    }
    return _checkmarkSpriteRenderer;
}

void UICheckbox::setStyle(const Style& style) {
    _style = style;
    applyVisualState();
}

UICheckbox::Style UICheckbox::getStyle() const {
    return _style;
}

void UICheckbox::applyVisualState() {
    if (getBackgroundSpriteRenderer() == nullptr) {
        return;
    }

    if (getCheckmarkSpriteRenderer() != nullptr) {
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

void UICheckbox::init(Style style, bool value, std::vector<std::pair<UIEventType, std::string>> events) {
    _style = style;
    _value = value;
    for (const auto& [eventType, actionId] : events) {
        addEventListener(eventType, actionId);
    }
}
} // namespace dzemikk
