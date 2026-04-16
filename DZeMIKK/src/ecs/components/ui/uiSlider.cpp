#include "ecs/components/ui/uiSlider.h"

#include "ecs/components/ui/rectTransform.h"
#include "ecs/components/ui/uiSliderActionRegistry.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/gameobject.h"

#include <algorithm>
#include <glm/common.hpp>
#include <glm/matrix.hpp>

namespace dzemikk {
namespace {
constexpr float kMinDeterminant = 1e-6F;
constexpr float kValueEpsilon = 1e-6F;

[[nodiscard]] bool tryPointToLocalX(const RectTransform* rectTransform, const glm::vec2& point,
                                    float& outLocalX) {
    if (rectTransform == nullptr) {
        return false;
    }

    const glm::mat4 worldMatrix = rectTransform->getWorldMatrix();
    const float determinant = glm::determinant(worldMatrix);
    if (glm::abs(determinant) < kMinDeterminant) {
        return false;
    }

    const glm::vec4 local = glm::inverse(worldMatrix) * glm::vec4(point, 0.0F, 1.0F);
    outLocalX = local[0];
    return true;
}

[[nodiscard]] bool containsPointInRect(const RectTransform* rectTransform, const glm::vec2& point) {
    if (rectTransform == nullptr) {
        return false;
    }

    const glm::mat4 worldMatrix = rectTransform->getWorldMatrix();
    const float determinant = glm::determinant(worldMatrix);
    if (glm::abs(determinant) < kMinDeterminant) {
        return false;
    }

    const glm::vec4 local = glm::inverse(worldMatrix) * glm::vec4(point, 0.0F, 1.0F);
    return local[0] >= 0.0F && local[0] <= 1.0F && local[1] >= 0.0F && local[1] <= 1.0F;
}

[[nodiscard]] float pointToNormalizedX(const RectTransform* rectTransform, const glm::vec2& point) {
    float localX = 0.0F;
    if (!tryPointToLocalX(rectTransform, point, localX)) {
        return 0.0F;
    }
    return std::clamp(localX, 0.0F, 1.0F);
}
}

bool UISlider::containsPoint(const glm::vec2& point) const {
    const RectTransform* handle = handleRect();
    if (handle != nullptr) {
        return containsPointInRect(handle, point);
    }

    return containsPointInRect(trackRect(), point);
}

void UISlider::processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                              bool releasedThisFrame) {
    _pointerDown = isDown;
    _pointerInside = containsPoint(point);

    if (_pointerInside && !_hovered) {
        _hovered = true;
        onEnter();
    } else if (!_pointerInside && _hovered) {
        _hovered = false;
        onExit();
    }

    processPress(point, pressedThisFrame);
    processRelease(releasedThisFrame);
    processDrag(point);
    applyVisualState();
}

const RectTransform* UISlider::handleRect() const {
    return _handleSpriteRenderer ? _handleSpriteRenderer->getRectTransform() : nullptr;
}

const RectTransform* UISlider::backgroundRect() const {
    return _backgroundSpriteRenderer ? _backgroundSpriteRenderer->getRectTransform() : nullptr;
}

const RectTransform* UISlider::fillRect() const {
    return _fillSpriteRenderer ? _fillSpriteRenderer->getRectTransform() : nullptr;
}

const RectTransform* UISlider::trackRect() const {
    if (_owner == nullptr) {
        return nullptr;
    }
    return _owner->rectTransform();
}

void UISlider::processPress(const glm::vec2& point, bool pressedThisFrame) {
    if (!pressedThisFrame) {
        return;
    }

    const bool pressedOnHandle = containsPointInRect(handleRect(), point);
    _pressedInside = pressedOnHandle;
    if (pressedOnHandle) {
        return;
    }

    const RectTransform* snapRect = backgroundRect();
    if (snapRect == nullptr) {
        snapRect = trackRect();
    }

    if (snapRect == nullptr || !containsPointInRect(snapRect, point)) {
        return;
    }

    const float normalized = pointToNormalizedX(snapRect, point);
    const float nextValue = _minValue + ((_maxValue - _minValue) * normalized);
    if (glm::abs(nextValue - _value) > kValueEpsilon) {
        onValueChanged(nextValue);
    }
}

void UISlider::processRelease(bool releasedThisFrame) {
    if (!releasedThisFrame) {
        return;
    }

    if (_pressedInside && _pointerInside) {
        onClick();
    }
    _pressedInside = false;
}

void UISlider::processDrag(const glm::vec2& point) {
    if (!_pressedInside || !_pointerDown) {
        return;
    }

    const RectTransform* dragRect = backgroundRect();
    if (dragRect == nullptr) {
        dragRect = trackRect();
    }

    if (dragRect == nullptr) {
        return;
    }

    const float normalized = pointToNormalizedX(dragRect, point);
    const float nextValue = _minValue + ((_maxValue - _minValue) * normalized);
    if (glm::abs(nextValue - _value) > kValueEpsilon) {
        onValueChanged(nextValue);
    }
}

void UISlider::onClick() {
    if (!_onClick && !_onClickActionId.empty()) {
        tryBindActionsFromIds();
    }

    if (_onClick) {
        _onClick();
    }
}

void UISlider::onEnter() {
    if (!_onEnter && !_onEnterActionId.empty()) {
        tryBindActionsFromIds();
    }

    if (_onEnter) {
        _onEnter();
    }
}

void UISlider::onExit() {
    if (!_onExit && !_onExitActionId.empty()) {
        tryBindActionsFromIds();
    }

    if (_onExit) {
        _onExit();
    }
}

void UISlider::onValueChanged(float newValue) {
    const float minBound = std::min(_minValue, _maxValue);
    const float maxBound = std::max(_minValue, _maxValue);
    _value = std::clamp(newValue, minBound, maxBound);

    if (!_onValueChanged && !_onValueChangedActionId.empty()) {
        tryBindActionsFromIds();
    }

    if (_onValueChanged) {
        _onValueChanged(_value);
    }
}

float UISlider::getValue() const {
    return _value;
}

void UISlider::setOnClick(std::function<void()> onClick) {
    _onClick = std::move(onClick);
}

void UISlider::setOnEnter(std::function<void()> onEnter) {
    _onEnter = std::move(onEnter);
}

void UISlider::setOnExit(std::function<void()> onExit) {
    _onExit = std::move(onExit);
}

void UISlider::setOnValueChanged(std::function<void(float)> onValueChanged) {
    _onValueChanged = std::move(onValueChanged);
}

void UISlider::setBackgroundSpriteRenderer(ImageRenderer* spriteRenderer) {
    _backgroundSpriteRenderer = spriteRenderer;
    applyVisualState();
}

void UISlider::setFillSpriteRenderer(ImageRenderer* spriteRenderer) {
    _fillSpriteRenderer = spriteRenderer;
    applyVisualState();
}

void UISlider::setHandleSpriteRenderer(ImageRenderer* spriteRenderer) {
    _handleSpriteRenderer = spriteRenderer;
    applyVisualState();
}

void UISlider::setFillColor(const glm::vec4& color) {
    _fillColor = color;
    applyVisualState();
}

void UISlider::setBackgroundColor(const glm::vec4& color) {
    _backgroundColor = color;
    applyVisualState();
}

void UISlider::setHandleColor(const glm::vec4& color) {
    _handleColor = color;
    applyVisualState();
}

void UISlider::setHandleHoverColor(const glm::vec4& color) {
    _handleHoverColor = color;
    applyVisualState();
}

void UISlider::setHandlePressedColor(const glm::vec4& color) {
    _handlePressedColor = color;
    applyVisualState();
}

void UISlider::setOnClickActionId(std::string actionId) {
    _onClickActionId = std::move(actionId);
    if (_onClick == nullptr) {
        tryBindActionsFromIds();
    }
}

void UISlider::setOnEnterActionId(std::string actionId) {
    _onEnterActionId = std::move(actionId);
    if (_onEnter == nullptr) {
        tryBindActionsFromIds();
    }
}

void UISlider::setOnExitActionId(std::string actionId) {
    _onExitActionId = std::move(actionId);
    if (_onExit == nullptr) {
        tryBindActionsFromIds();
    }
}

void UISlider::setOnValueChangedActionId(std::string actionId) {
    _onValueChangedActionId = std::move(actionId);
    if (_onValueChanged == nullptr) {
        tryBindActionsFromIds();
    }
}

void UISlider::tryBindActionsFromIds() {
    auto& registry = UISliderActionRegistry::get();

    if (_onClick == nullptr && !_onClickActionId.empty()) {
        _onClick = registry.bind(_onClickActionId, *this);
    }

    if (_onEnter == nullptr && !_onEnterActionId.empty()) {
        _onEnter = registry.bind(_onEnterActionId, *this);
    }

    if (_onExit == nullptr && !_onExitActionId.empty()) {
        _onExit = registry.bind(_onExitActionId, *this);
    }

    if (_onValueChanged == nullptr && !_onValueChangedActionId.empty()) {
        _onValueChanged = registry.bindValueChanged(_onValueChangedActionId, *this);
    }
}

void UISlider::applyVisualState() {
    if (_backgroundSpriteRenderer != nullptr) {
        _backgroundSpriteRenderer->setColor(_backgroundColor);
    }

    if (_fillSpriteRenderer != nullptr) {
        _fillSpriteRenderer->setColor(_fillColor);
    }

    if (_handleSpriteRenderer == nullptr) {
        return;
    }

    if (_pressedInside && _pointerInside && _pointerDown) {
        _handleSpriteRenderer->setColor(_handlePressedColor);
    } else if (_hovered) {
        _handleSpriteRenderer->setColor(_handleHoverColor);
    } else {
        _handleSpriteRenderer->setColor(_handleColor);
    }
}
} // namespace dzemikk
