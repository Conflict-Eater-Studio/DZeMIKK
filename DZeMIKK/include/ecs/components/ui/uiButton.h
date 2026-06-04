#ifndef DZEMIKK_UIBUTTON_H
#define DZEMIKK_UIBUTTON_H

#include "assetManager/assetHandle.h"
#include "ecs/components/ui/iUIInteractable.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>

namespace dzemikk {
class ImageRenderer;
class Font;

class UIButton : public IUIInteractable {
  public:
    using Base = IUIInteractable;

    struct Style {
        glm::vec4 normalColor;
        glm::vec4 hoverColor;
        glm::vec4 pressedColor;
    };

    UIButton() = default;
    UIButton(const UIButton& other) = delete;
    UIButton& operator=(const UIButton& other) = delete;
    UIButton(UIButton&& other) noexcept = delete;
    UIButton& operator=(UIButton&& other) noexcept = delete;
    ~UIButton() override = default;

    [[nodiscard]] std::string typeName() const override {
        return "UIButton";
    }

    void processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                        bool releasedThisFrame, double scrollDelta) override;

    void setStyle(const Style& style);
    Style getStyle() const;

    [[nodiscard]] GameObject* getTextGO() const;
    [[nodiscard]] ImageRenderer* getSpriteRenderer() const;

    void applyVisualState();
    void init(Style style, std::vector<std::pair<UIEventType, std::string>> events);

  private:
    Style _style{.normalColor = glm::vec4(1.0F),
                 .hoverColor = glm::vec4(0.9F, 0.9F, 0.9F, 1.0F),
                 .pressedColor = glm::vec4(0.8F, 0.8F, 0.8F, 1.0F)};

    mutable ImageRenderer* _spriteRenderer = nullptr;
};
} // namespace dzemikk

#endif // DZEMIKK_UIBUTTON_H
