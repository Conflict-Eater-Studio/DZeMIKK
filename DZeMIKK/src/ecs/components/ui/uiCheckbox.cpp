#include "ecs/components/ui/uiCheckbox.h"

#include "ecs/components/ui/rectTransform.h"
#include "ecs/components/ui/uiCheckboxActionRegistry.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "spdlog/spdlog.h"

#include <glm/geometric.hpp>
#include <glm/matrix.hpp>

namespace dzemikk {
namespace {
constexpr float kMinDeterminant = 1e-6F;
}
bool UICheckbox::containsPoint(const glm::vec2& point) const {
    if (!_owner) {
        return false;
    }

    RectTransform* rectTransform = _owner->rectTransform();
    if (!rectTransform) {
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

void UICheckbox::processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
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

    if (pressedThisFrame) {
        _pressedInside = _pointerInside;
    }

    if (releasedThisFrame) {
        if (_pressedInside && _pointerInside) {
            onClick();
        }
        _pressedInside = false;
    }

    applyVisualState();
}

void UICheckbox::onClick() {
    if (!_onClick && !_onClickActionId.empty()) {
        tryBindActionsFromIds();
    }

    _value = !_value;
    _checkmarkSpriteRenderer->enabled(_value);

    if (_onClick) {
        _onClick();
    }
}

void UICheckbox::onEnter() {
    if (!_onEnter && !_onEnterActionId.empty()) {
        tryBindActionsFromIds();
    }

    if (_onEnter) {
        _onEnter();
    }
}

void UICheckbox::onExit() {
    if (!_onExit && !_onExitActionId.empty()) {
        tryBindActionsFromIds();
    }

    if (_onExit) {
        _onExit();
    }
}

void UICheckbox::setBackgroundSpriteRenderer(ImageRenderer* spriteRenderer) {
    _backgroundSpriteRenderer = spriteRenderer;
    applyVisualState();
}

void UICheckbox::setCheckmarkSpriteRenderer(ImageRenderer* spriteRenderer) {
    _checkmarkSpriteRenderer = spriteRenderer;
    _checkmarkSpriteRenderer->enabled(_value);
    applyVisualState();
}

void UICheckbox::setNormalColor(const glm::vec4& color) {
    _normalColor = color;
    applyVisualState();
}

void UICheckbox::setHoverColor(const glm::vec4& color) {
    _hoverColor = color;
    applyVisualState();
}

void UICheckbox::setPressedColor(const glm::vec4& color) {
    _pressedColor = color;
    applyVisualState();
}

void UICheckbox::setOnClick(std::function<void()> onClick) {
    _onClick = std::move(onClick);
}

void UICheckbox::setOnEnter(std::function<void()> onEnter) {
    _onEnter = std::move(onEnter);
}

void UICheckbox::setOnExit(std::function<void()> onExit) {
    _onExit = std::move(onExit);
}

void UICheckbox::setOnClickActionId(std::string actionId) {
    _onClickActionId = std::move(actionId);
    if (_onClick == nullptr) {
        tryBindActionsFromIds();
    }
}

void UICheckbox::setOnEnterActionId(std::string actionId) {
    _onEnterActionId = std::move(actionId);
    if (_onEnter == nullptr) {
        tryBindActionsFromIds();
    }
}

void UICheckbox::setOnExitActionId(std::string actionId) {
    _onExitActionId = std::move(actionId);
    if (_onExit == nullptr) {
        tryBindActionsFromIds();
    }
}

void UICheckbox::tryBindActionsFromIds() {
    auto& registry = UICheckboxActionRegistry::get();

    if (_onClick == nullptr && !_onClickActionId.empty()) {
        _onClick = std::move(registry.bind(_onClickActionId, *this));
    }

    if (_onEnter == nullptr && !_onEnterActionId.empty()) {
        _onEnter = std::move(registry.bind(_onEnterActionId, *this));
    }

    if (_onExit == nullptr && !_onExitActionId.empty()) {
        _onExit = std::move(registry.bind(_onExitActionId, *this));
    }
}

void UICheckbox::applyVisualState() {
    if (!_backgroundSpriteRenderer) {
        return;
    }

    if (_pressedInside && _pointerInside && _pointerDown) {
        _backgroundSpriteRenderer->setColor(_pressedColor);
    } else if (_hovered) {
        _backgroundSpriteRenderer->setColor(_hoverColor);
    } else {
        _backgroundSpriteRenderer->setColor(_normalColor);
    }
}
} // namespace dzemikk
