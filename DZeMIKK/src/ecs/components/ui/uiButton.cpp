#include "ecs/components/ui/uiButton.h"

#include "ecs/components/ui/rectTransform.h"
#include "ecs/components/ui/uiButtonActionRegistry.h"
#include "ecs/components/ui/uiSpriteRenderer.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "spdlog/spdlog.h"

#include <glm/geometric.hpp>
#include <glm/matrix.hpp>


namespace dzemikk {
namespace {
constexpr float kMinDeterminant = 1e-6F;
}
void UIButton::build(GameObject& gameObject, const UIButtonParams& params) {
    auto* button = gameObject.addComponent<UIButton>();
    button->setNormalColor(params.normalColor);
    button->setHoverColor(params.hoverColor);
    button->setPressedColor(params.pressedColor);
    button->setOnClickActionId(params.onClickActionId);
    button->setOnEnterActionId(params.onEnterActionId);
    button->setOnExitActionId(params.onExitActionId);
    button->setOnClick(params.onClick);
    button->setOnEnter(params.onEnter);
    button->setOnExit(params.onExit);

    auto* rectTransform = gameObject.rectTransform();
    rectTransform->setSize(params.rectTransformParams.size);
    rectTransform->setAnchorMin(params.rectTransformParams.anchorMin);
    rectTransform->setAnchorMax(params.rectTransformParams.anchorMax);
    rectTransform->setPivot(params.rectTransformParams.pivot);
    rectTransform->setPosition(params.rectTransformParams.position);
    rectTransform->setScale(params.rectTransformParams.scale);
    rectTransform->setRotation(params.rectTransformParams.rotation);

    auto* spriteRenderer = gameObject.addComponent<UISpriteRenderer>();
    spriteRenderer->setMesh(params.mesh);
    spriteRenderer->setMaterial(params.material);
    spriteRenderer->setRectTransform(rectTransform);
    spriteRenderer->setColor(params.normalColor);

    button->setSpriteRenderer(spriteRenderer);
}

bool UIButton::containsPoint(const glm::vec2& point) const {
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

void UIButton::processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
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

void UIButton::onClick() {
    if (!_onClick && !_onClickActionId.empty()) {
        tryBindActionsFromIds();
    }

    if (_onClick) {
        _onClick();
    }
}

void UIButton::onEnter() {
    if (!_onEnter && !_onEnterActionId.empty()) {
        tryBindActionsFromIds();
    }

    if (_onEnter) {
        _onEnter();
    }
}

void UIButton::onExit() {
    if (!_onExit && !_onExitActionId.empty()) {
        tryBindActionsFromIds();
    }

    if (_onExit) {
        _onExit();
    }
}

void UIButton::setSpriteRenderer(UISpriteRenderer* spriteRenderer) {
    _spriteRenderer = spriteRenderer;
    applyVisualState();
}

void UIButton::setNormalColor(const glm::vec4& color) {
    _normalColor = color;
    applyVisualState();
}

void UIButton::setHoverColor(const glm::vec4& color) {
    _hoverColor = color;
    applyVisualState();
}

void UIButton::setPressedColor(const glm::vec4& color) {
    _pressedColor = color;
    applyVisualState();
}

void UIButton::setOnClick(std::function<void()> onClick) {
    _onClick = std::move(onClick);
}

void UIButton::setOnEnter(std::function<void()> onEnter) {
    _onEnter = std::move(onEnter);
}

void UIButton::setOnExit(std::function<void()> onExit) {
    _onExit = std::move(onExit);
}

void UIButton::setOnClickActionId(std::string actionId) {
    _onClickActionId = std::move(actionId);
    if (_onClick == nullptr) {
        tryBindActionsFromIds();
    }
}

void UIButton::setOnEnterActionId(std::string actionId) {
    _onEnterActionId = std::move(actionId);
    if (_onEnter == nullptr) {
        tryBindActionsFromIds();
    }
}

void UIButton::setOnExitActionId(std::string actionId) {
    _onExitActionId = std::move(actionId);
    if (_onExit == nullptr) {
        tryBindActionsFromIds();
    }
}

void UIButton::tryBindActionsFromIds() {
    auto& registry = UIButtonActionRegistry::get();

    if (_onClick == nullptr && !_onClickActionId.empty()) {
        _onClick = registry.bind(_onClickActionId, *this);
    }

    if (_onEnter == nullptr && !_onEnterActionId.empty()) {
        _onEnter = registry.bind(_onEnterActionId, *this);
    }

    if (_onExit == nullptr && !_onExitActionId.empty()) {
        _onExit = registry.bind(_onExitActionId, *this);
    }
}

void UIButton::applyVisualState() {
    if (!_spriteRenderer) {
        return;
    }

    if (_pressedInside && _pointerInside && _pointerDown) {
        _spriteRenderer->setColor(_pressedColor);
    } else if (_hovered) {
        _spriteRenderer->setColor(_hoverColor);
    } else {
        _spriteRenderer->setColor(_normalColor);
    }
}
} // namespace dzemikk
