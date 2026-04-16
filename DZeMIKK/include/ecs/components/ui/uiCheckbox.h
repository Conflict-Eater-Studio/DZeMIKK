#ifndef DZEMIKK_UICHECKBOX_H
#define DZEMIKK_UICHECKBOX_H

#include "ecs/components/ui/iUIInteractable.h"
#include "ecs/components/ui/rectTransform.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

#include <functional>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>

namespace dzemikk {
class ImageRenderer;
class Font;

class UICheckbox : public IUIInteractable {
  public:
    using Base = IUIInteractable;

    UICheckbox() = default;
    UICheckbox(const UICheckbox& other) = delete;
    UICheckbox& operator=(const UICheckbox& other) = delete;
    UICheckbox(UICheckbox&& other) noexcept = delete;
    UICheckbox& operator=(UICheckbox&& other) noexcept = delete;
    ~UICheckbox() override = default;

    [[nodiscard]] std::string typeName() const override {
        return "UICheckbox";
    }

    [[nodiscard]] bool containsPoint(const glm::vec2& point) const override;
    void processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                        bool releasedThisFrame) override;

    [[nodiscard]] bool isHovered() const override {
        return _hovered;
    }

    [[nodiscard]] bool isPressed() const override {
        return _pressedInside;
    }

    void onClick() override;
    void onEnter() override;
    void onExit() override;

    void setBackgroundSpriteRenderer(ImageRenderer* spriteRenderer);
    void setCheckmarkSpriteRenderer(ImageRenderer* spriteRenderer);

    void setNormalColor(const glm::vec4& color);
    void setHoverColor(const glm::vec4& color);
    void setPressedColor(const glm::vec4& color);
    [[nodiscard]] const glm::vec4& getNormalColor() const {
        return _normalColor;
    }
    [[nodiscard]] const glm::vec4& getHoverColor() const {
        return _hoverColor;
    }
    [[nodiscard]] const glm::vec4& getPressedColor() const {
        return _pressedColor;
    }

    void setOnClick(std::function<void()> onClick);
    void setOnEnter(std::function<void()> onEnter);
    void setOnExit(std::function<void()> onExit);

    void setOnClickActionId(std::string actionId);
    void setOnEnterActionId(std::string actionId);
    void setOnExitActionId(std::string actionId);
    [[nodiscard]] const std::string& getOnClickActionId() const {
        return _onClickActionId;
    }
    [[nodiscard]] const std::string& getOnEnterActionId() const {
        return _onEnterActionId;
    }
    [[nodiscard]] const std::string& getOnExitActionId() const {
        return _onExitActionId;
    }
    [[nodiscard]] bool getValue() const {
        return _value;
    }

  private:
    void tryBindActionsFromIds();

    void applyVisualState();

    mutable ImageRenderer* _backgroundSpriteRenderer = nullptr;
    mutable ImageRenderer* _checkmarkSpriteRenderer = nullptr;

    glm::vec4 _normalColor = glm::vec4(1.0F);
    glm::vec4 _hoverColor = glm::vec4(1.0F, 1.0F, 1.0F, 0.85F);
    glm::vec4 _pressedColor = glm::vec4(0.75F, 0.75F, 0.75F, 1.0F);

    bool _hovered = false;
    bool _pressedInside = false;
    bool _pointerInside = false;
    bool _pointerDown = false;

    std::string _onClickActionId;
    std::string _onEnterActionId;
    std::string _onExitActionId;

    std::function<void()> _onClick;
    std::function<void()> _onEnter;
    std::function<void()> _onExit;

    bool _value = false;
};
} // namespace dzemikk

#endif // DZEMIKK_UICHECKBOX_H
