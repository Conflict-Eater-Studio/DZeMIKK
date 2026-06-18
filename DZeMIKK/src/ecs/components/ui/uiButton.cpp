#include "ecs/components/ui/uiButton.h"

#include "assetManager/primitiveMeshLibrary.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/rectTransform.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <glm/matrix.hpp>

namespace dzemikk {
namespace {
constexpr float kMinDeterminant = 1e-6F;
}

void UIButton::processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                              bool releasedThisFrame, double scrollDelta) {
    (void)scrollDelta;

    setPointerDown(isDown);
    setPointerInside(_owner != nullptr && _owner->rectTransform() != nullptr &&
                     _owner->rectTransform()->containsPoint(point));
    if (isInteractable()) {
        updateHoverState();
        processStandardPressRelease(pressedThisFrame, releasedThisFrame);
    }
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
    if (getSpriteRenderer() == nullptr) {
        return;
    }

    if (!isInteractable()) {
        _spriteRenderer->setColor(_style.disabledColor);
    } else if (pressedInside() && pointerInside() && pointerDown()) {
        _spriteRenderer->setColor(_style.pressedColor);
    } else if (isHovered()) {
        _spriteRenderer->setColor(_style.hoverColor);
    } else {
        _spriteRenderer->setColor(_style.normalColor);
    }
}

GameObject* UIButton::getTextGO() const {
    if (_owner == nullptr || _owner->getChildren().empty()) {
        return nullptr;
    }
    return _owner->getChildren().front();
}

ImageRenderer* UIButton::getSpriteRenderer() const {
    if (_spriteRenderer == nullptr && _owner != nullptr) {
        _spriteRenderer = _owner->getComponent<ImageRenderer>();
    }

    return _spriteRenderer;
}

void UIButton::init(Style style, std::vector<std::pair<UIEventType, std::string>> events) {
    _style = style;
    for (const auto& [eventType, actionId] : events) {
        addEventListener(eventType, actionId);
    }
}
} // namespace dzemikk
