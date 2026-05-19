#ifndef DZEMIKK_UICHECKBOX_H
#define DZEMIKK_UICHECKBOX_H

#include "ecs/components/ui/iUIInteractable.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/gameobject.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>

namespace dzemikk {
class ImageRenderer;
class Font;

class UICheckbox : public IUIInteractable {
  public:
    using Base = IUIInteractable;

    struct Style {
        glm::vec4 normalColor = glm::vec4(1.0F);
        glm::vec4 hoverColor = glm::vec4(1.0F, 1.0F, 1.0F, 0.85F);
        glm::vec4 pressedColor = glm::vec4(0.75F, 0.75F, 0.75F, 1.0F);
        glm::vec4 checkmarkColor = glm::vec4(0.2F, 0.6F, 1.0F, 1.0F);
    };

    UICheckbox() = default;
    UICheckbox(const UICheckbox& other) = delete;
    UICheckbox& operator=(const UICheckbox& other) = delete;
    UICheckbox(UICheckbox&& other) noexcept = delete;
    UICheckbox& operator=(UICheckbox&& other) noexcept = delete;
    ~UICheckbox() override = default;

    [[nodiscard]] std::string typeName() const override {
        return "UICheckbox";
    }

    void processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                        bool releasedThisFrame, double scrollDelta) override;

    void onClick() override;

    [[nodiscard]] ImageRenderer* getBackgroundSpriteRenderer() const;
    [[nodiscard]] ImageRenderer* getCheckmarkSpriteRenderer() const;

    void setStyle(const Style& style);
    [[nodiscard]] Style getStyle() const;

  private:
    void applyVisualState();

    mutable ImageRenderer* _backgroundSpriteRenderer = nullptr;
    mutable ImageRenderer* _checkmarkSpriteRenderer = nullptr;
    Style _style;
    bool _value = false;
};
} // namespace dzemikk

#endif // DZEMIKK_UICHECKBOX_H
