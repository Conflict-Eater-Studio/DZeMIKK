#ifndef DZEMIKK_UISLIDER_H
#define DZEMIKK_UISLIDER_H

#include "ecs/components/ui/iUIInteractable.h"

#include <glm/vec4.hpp>
#include <string>

namespace dzemikk {
class ImageRenderer;
class RectTransform;

class UISlider : public IUIInteractable {
  public:
    using Base = IUIInteractable;

    struct Style {
        glm::vec4 fillColor = glm::vec4(0.2F, 0.5F, 1.0F, 1.0F);
        glm::vec4 backgroundColor = glm::vec4(0.18F, 0.2F, 0.24F, 1.0F);
        glm::vec4 handleColor = glm::vec4(0.9F, 0.92F, 0.98F, 1.0F);
        glm::vec4 handleHoverColor = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
        glm::vec4 handlePressedColor = glm::vec4(0.7F, 0.78F, 0.9F, 1.0F);
    };

    UISlider() = default;
    UISlider(const UISlider& other) = delete;
    UISlider& operator=(const UISlider& other) = delete;
    UISlider(UISlider&& other) noexcept = delete;
    UISlider& operator=(UISlider&& other) noexcept = delete;
    ~UISlider() override = default;

    [[nodiscard]] std::string typeName() const override {
        return "UISlider";
    }

    void processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                        bool releasedThisFrame, double scrollDelta) override;

    void onValueChanged(float newValue);
    [[nodiscard]] float getValue() const;

    [[nodiscard]] ImageRenderer* getBackgroundSpriteRenderer() const;
    [[nodiscard]] ImageRenderer* getFillSpriteRenderer() const;
    [[nodiscard]] ImageRenderer* getHandleSpriteRenderer() const;

    void setStyle(const Style& style);
    [[nodiscard]] const Style& getStyle() const {
        return _style;
    }

    void setStep(float step) {
        _step = step;
    }
    [[nodiscard]] float getStep() const {
        return _step;
    }
    void setMinValue(float minValue) {
        _minValue = minValue;
    }
    [[nodiscard]] float getMinValue() const {
        return _minValue;
    }
    void setMaxValue(float maxValue) {
        _maxValue = maxValue;
    }
    [[nodiscard]] float getMaxValue() const {
        return _maxValue;
    }

  private:
    void processPress(const glm::vec2& point, bool pressedThisFrame);
    void processRelease(bool releasedThisFrame);
    void processDrag(const glm::vec2& point);
    void processScroll(double scrollDelta);
    void applyVisualState();

    float _value = 0.0F;
    float _minValue = 0.0F;
    float _maxValue = 1.0F;
    float _step = 0.01F;

    Style _style;

    mutable ImageRenderer* _backgroundSpriteRenderer = nullptr;
    mutable ImageRenderer* _fillSpriteRenderer = nullptr;
    mutable ImageRenderer* _handleSpriteRenderer = nullptr;
};
} // namespace dzemikk

#endif // DZEMIKK_UISLIDER_H
