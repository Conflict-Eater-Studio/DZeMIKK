#ifndef DZEMIKK_UIBUTTON_H
#define DZEMIKK_UIBUTTON_H

#include "ecs/components/ui/iUIInteractable.h"
#include "ecs/components/ui/rectTransform.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

#include <functional>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>

namespace dzemikk {
class UISpriteRenderer;
class Font;

struct UIButtonParams {
    glm::vec4 normalColor = glm::vec4(1.0F);
    glm::vec4 hoverColor = glm::vec4(0.9F, 0.9F, 0.9F, 1.0F);
    glm::vec4 pressedColor = glm::vec4(0.8F, 0.8F, 0.8F, 1.0F);
    std::function<void()> onClick;
    std::function<void()> onEnter;
    std::function<void()> onExit;
    RectTransformParams rectTransformParams;
    Mesh* mesh;
    Material* material;
};

class UIButton : public IUIInteractable {
  public:
    using Base = IUIInteractable;

    UIButton() = default;
    UIButton(const UIButton& other) = delete;
    UIButton& operator=(const UIButton& other) = delete;
    UIButton(UIButton&& other) noexcept = delete;
    UIButton& operator=(UIButton&& other) noexcept = delete;
    ~UIButton() override = default;

    static void build(GameObject& gameObject, const UIButtonParams& params);

    [[nodiscard]] std::string typeName() const override {
        return "UIButton";
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

    void setSpriteRenderer(UISpriteRenderer* spriteRenderer);

    void setNormalColor(const glm::vec4& color);
    void setHoverColor(const glm::vec4& color);
    void setPressedColor(const glm::vec4& color);

    void setOnClick(std::function<void()> onClick);
    void setOnEnter(std::function<void()> onEnter);
    void setOnExit(std::function<void()> onExit);

  private:
    void applyVisualState();

    mutable UISpriteRenderer* _spriteRenderer = nullptr;

    glm::vec4 _normalColor = glm::vec4(1.0F);
    glm::vec4 _hoverColor = glm::vec4(1.0F, 1.0F, 1.0F, 0.85F);
    glm::vec4 _pressedColor = glm::vec4(0.75F, 0.75F, 0.75F, 1.0F);

    bool _hovered = false;
    bool _pressedInside = false;
    bool _pointerInside = false;
    bool _pointerDown = false;

    std::function<void()> _onClick;
    std::function<void()> _onEnter;
    std::function<void()> _onExit;
};
} // namespace dzemikk

#endif // DZEMIKK_UIBUTTON_H