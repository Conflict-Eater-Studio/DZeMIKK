#include "ecs/components/ui/uiSlider.h"

#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/rectTransform.h"
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

[[nodiscard]] float pointToNormalizedX(const RectTransform* rectTransform, const glm::vec2& point) {
    float localX = 0.0F;
    if (!tryPointToLocalX(rectTransform, point, localX)) {
        return 0.0F;
    }

    return std::clamp(localX, 0.0F, 1.0F);
}
} // namespace

void UISlider::processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                              bool releasedThisFrame, double scrollDelta) {
    setPointerDown(isDown);

    setPointerInside(getHandleSpriteRenderer()->getRectTransform() != nullptr
                         ? getHandleSpriteRenderer()->getRectTransform()->containsPoint(point)
                         : _owner->rectTransform()->containsPoint(point));
    updateHoverState();

    processPress(point, pressedThisFrame);
    processRelease(releasedThisFrame);
    processDrag(point);
    processScroll(scrollDelta);
    applyVisualState();
}

void UISlider::processPress(const glm::vec2& point, bool pressedThisFrame) {
    if (!pressedThisFrame) {
        return;
    }

    const bool pressedOnHandle = _handleSpriteRenderer->getRectTransform()->containsPoint(point);
    setPressedInside(pressedOnHandle);
    if (pressedOnHandle) {
        return;
    }

    const RectTransform* snapRect = _backgroundSpriteRenderer->getRectTransform();
    if (snapRect == nullptr) {
        snapRect = _backgroundSpriteRenderer->getRectTransform();
    }

    if (snapRect == nullptr || !snapRect->containsPoint(point)) {
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

    if (pressedInside() && pointerInside()) {
        onClick();
    }

    setPressedInside(false);
}

void UISlider::processDrag(const glm::vec2& point) {
    if (!pressedInside() || !pointerDown()) {
        return;
    }

    const RectTransform* dragRect = _backgroundSpriteRenderer->getRectTransform();
    if (dragRect == nullptr) {
        return;
    }

    const float normalized = pointToNormalizedX(dragRect, point);
    const float nextValue = _minValue + ((_maxValue - _minValue) * normalized);
    // Snap to step increments
    const float steppedValue = (std::round((nextValue - _minValue) / _step) * _step) + _minValue;
    if (glm::abs(steppedValue - _value) > kValueEpsilon) {
        onValueChanged(steppedValue);
    }
}

void UISlider::processScroll(double scrollDelta) {
    if (!pointerInside() && glm::abs(scrollDelta) < kValueEpsilon) {
        return;
    }

    const auto deltaValue = static_cast<float>(scrollDelta);
    onValueChanged(_value + (_step * glm::sign(deltaValue)));
}

void UISlider::onValueChanged(float newValue) {
    if (newValue == _value) {
        return;
    }

    const float minBound = std::min(_minValue, _maxValue);
    const float maxBound = std::max(_minValue, _maxValue);
    _value = std::clamp(newValue, minBound, maxBound);

    auto* fillRect = _fillSpriteRenderer->getRectTransform();
    fillRect->setPivot({0.0f, 0.5f});
    fillRect->setPosition(
        {-_backgroundSpriteRenderer->getOwner()->rectTransform()->getSize().x / 2,
         fillRect->getPosition().y});

    if (fillRect != nullptr) {
        float t = (_value - _minValue) / (_maxValue - _minValue);
        t = glm::clamp(t, 0.0f, 1.0f);

        glm::vec2 scale = fillRect->getScale();
        scale.x = t;
        fillRect->setScale(scale);
    }

    auto slideArea = _fillSpriteRenderer->getRectTransform()->getSize();
    auto handleProgress = (_value - _minValue) / (_maxValue - _minValue);
    auto handlePosX = (handleProgress * slideArea[0]) - (slideArea[0] * 0.5F);
    _handleSpriteRenderer->getRectTransform()->setPosition({handlePosX, 0.0F});

    emit(UIEventType::ValueChanged, _value);
}

float UISlider::getValue() const {
    return _value;
}

ImageRenderer* UISlider::getBackgroundSpriteRenderer() const {
    if (_backgroundSpriteRenderer == nullptr) {
        _backgroundSpriteRenderer = _owner->getComponent<ImageRenderer>();
    }

    return _backgroundSpriteRenderer;
}

ImageRenderer* UISlider::getFillSpriteRenderer() const {
    if (_fillSpriteRenderer == nullptr) {
        for (const auto& child : _owner->getChildren()) {
            if (child == nullptr || child->getName().find("_Fill") == std::string::npos) {
                continue;
            }

            auto* img = child->getComponent<ImageRenderer>();
            if (img != nullptr) {
                _fillSpriteRenderer = img;
                break;
            }
        }
    }

    return _fillSpriteRenderer;
}

ImageRenderer* UISlider::getHandleSpriteRenderer() const {
    if (_handleSpriteRenderer == nullptr) {
        for (const auto& child : _owner->getChildren()) {
            if (child == nullptr || child->getName().find("_Handle") == std::string::npos) {
                continue;
            }

            auto* img = child->getComponent<ImageRenderer>();
            if (img != nullptr) {
                _handleSpriteRenderer = img;
                break;
            }
        }
    }

    return _handleSpriteRenderer;
}

void UISlider::setStyle(const Style& style) {
    _style = style;
    applyVisualState();
}

void UISlider::applyVisualState() {
    if (getBackgroundSpriteRenderer() != nullptr) {
        _backgroundSpriteRenderer->setColor(_style.backgroundColor);
    }

    if (getFillSpriteRenderer() != nullptr) {
        _fillSpriteRenderer->setColor(_style.fillColor);

        auto* fillRect = _fillSpriteRenderer->getRectTransform();
        fillRect->setPivot({0.0f, 0.5f});
        fillRect->setPosition(
            {-_backgroundSpriteRenderer->getOwner()->rectTransform()->getSize().x / 2,
             fillRect->getPosition().y});

        if (fillRect != nullptr) {
            float t = (_value - _minValue) / (_maxValue - _minValue);
            t = glm::clamp(t, 0.0f, 1.0f);

            glm::vec2 scale = fillRect->getScale();
            scale.x = t;
            fillRect->setScale(scale);
        }
    }

    if (getHandleSpriteRenderer() == nullptr) {
        return;
    }

    if (pressedInside() && pointerInside() && pointerDown()) {
        _handleSpriteRenderer->setColor(_style.handlePressedColor);
    } else if (isHovered()) {
        _handleSpriteRenderer->setColor(_style.handleHoverColor);
    } else {
        _handleSpriteRenderer->setColor(_style.handleColor);
    }
}

void UISlider::init(Style style, float value, float minValue, float maxValue, float step,
                    std::vector<std::pair<UIEventType, std::string>> events) {
    _style = style;
    _value = std::clamp(value, std::min(minValue, maxValue), std::max(minValue, maxValue));
    _minValue = minValue;
    _maxValue = maxValue;
    _step = step;
    for (const auto& [eventType, actionId] : events) {
        addEventListener(eventType, actionId);
    }
}
} // namespace dzemikk
