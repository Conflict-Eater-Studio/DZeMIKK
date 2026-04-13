#ifndef DZEMIKK_UISLIDER_H
#define DZEMIKK_UISLIDER_H

#include "ecs/components/ui/iUIInteractable.h"

#include <functional>
#include <glm/vec4.hpp>
#include <string>

namespace dzemikk {
class UISpriteRenderer;
class RectTransform;

class UISlider : public IUIInteractable {
  public:
    using Base = IUIInteractable;

    UISlider() = default;
    UISlider(const UISlider& other) = delete;
    UISlider& operator=(const UISlider& other) = delete;
    UISlider(UISlider&& other) noexcept = delete;
    UISlider& operator=(UISlider&& other) noexcept = delete;
    ~UISlider() override = default;

    [[nodiscard]] std::string typeName() const override {
        return "UISlider";
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
    void onValueChanged(float newValue);
    [[nodiscard]] float getValue() const;

    void setOnClick(std::function<void()> onClick);
    void setOnEnter(std::function<void()> onEnter);
    void setOnExit(std::function<void()> onExit);
    void setOnValueChanged(std::function<void(float)> onValueChanged);

    void setBackgroundSpriteRenderer(UISpriteRenderer* spriteRenderer);
    void setFillSpriteRenderer(UISpriteRenderer* spriteRenderer);
    void setHandleSpriteRenderer(UISpriteRenderer* spriteRenderer);

    void setFillColor(const glm::vec4& color);
    void setBackgroundColor(const glm::vec4& color);
    void setHandleColor(const glm::vec4& color);
    void setHandleHoverColor(const glm::vec4& color);
    void setHandlePressedColor(const glm::vec4& color);

    [[nodiscard]] const glm::vec4& getFillColor() const {
        return _fillColor;
    }
    [[nodiscard]] const glm::vec4& getBackgroundColor() const {
        return _backgroundColor;
    }
    [[nodiscard]] const glm::vec4& getHandleColor() const {
        return _handleColor;
    }
    [[nodiscard]] const glm::vec4& getHandleHoverColor() const {
        return _handleHoverColor;
    }
    [[nodiscard]] const glm::vec4& getHandlePressedColor() const {
        return _handlePressedColor;
    }

    void setOnClickActionId(std::string actionId);
    void setOnEnterActionId(std::string actionId);
    void setOnExitActionId(std::string actionId);
    void setOnValueChangedActionId(std::string actionId);

    [[nodiscard]] const std::string& getOnClickActionId() const {
        return _onClickActionId;
    }
    [[nodiscard]] const std::string& getOnEnterActionId() const {
        return _onEnterActionId;
    }
    [[nodiscard]] const std::string& getOnExitActionId() const {
        return _onExitActionId;
    }
    [[nodiscard]] const std::string& getOnValueChangedActionId() const {
        return _onValueChangedActionId;
    }

  private:
    void tryBindActionsFromIds();
    [[nodiscard]] const RectTransform* handleRect() const;
    [[nodiscard]] const RectTransform* backgroundRect() const;
    [[nodiscard]] const RectTransform* fillRect() const;
    [[nodiscard]] const RectTransform* trackRect() const;

    void processPress(const glm::vec2& point, bool pressedThisFrame);
    void processRelease(bool releasedThisFrame);
    void processDrag(const glm::vec2& point);
    void applyVisualState();

    float _value = 0.0F;
    float _minValue = 0.0F;
    float _maxValue = 1.0F;

    bool _hovered = false;
    bool _pressedInside = false;
    bool _pointerInside = false;
    bool _pointerDown = false;

    glm::vec4 _fillColor = glm::vec4(0.2F, 0.5F, 1.0F, 1.0F);
    glm::vec4 _backgroundColor = glm::vec4(0.18F, 0.2F, 0.24F, 1.0F);
    glm::vec4 _handleColor = glm::vec4(0.9F, 0.92F, 0.98F, 1.0F);
    glm::vec4 _handleHoverColor = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
    glm::vec4 _handlePressedColor = glm::vec4(0.7F, 0.78F, 0.9F, 1.0F);

    std::string _onClickActionId;
    std::string _onEnterActionId;
    std::string _onExitActionId;
    std::string _onValueChangedActionId;

    std::function<void()> _onClick;
    std::function<void()> _onEnter;
    std::function<void()> _onExit;
    std::function<void(float)> _onValueChanged;

    mutable UISpriteRenderer* _backgroundSpriteRenderer = nullptr;
    mutable UISpriteRenderer* _fillSpriteRenderer = nullptr;
    mutable UISpriteRenderer* _handleSpriteRenderer = nullptr;
};
} // namespace dzemikk

#endif // DZEMIKK_UISLIDER_H
