#ifndef DZEMIKK_UIDROPDOWN_H
#define DZEMIKK_UIDROPDOWN_H

#include "ecs/components/ui/iUIInteractable.h"
#include "ecs/components/ui/uiButton.h"
#include "ecs/components/ui/uiTextRenderer.h"

#include <spdlog/spdlog.h>
#include <vector>

namespace dzemikk {
class ImageRenderer;
class Mesh;
class Material;
class UIDropdown : public IUIInteractable {
  public:
    using Base = IUIInteractable;

    struct OptionRender {
        float height{30.0F};
        AssetHandle<Mesh> mesh;
        std::shared_ptr<dzemikk::Material> material{nullptr};
        AssetHandle<Font> font;
        UITextRenderer::VerticalAlign textVAlign{UITextRenderer::VerticalAlign::Middle};
        UITextRenderer::HorizontalAlign textHAlign{UITextRenderer::HorizontalAlign::Center};
    };

    struct Option {
        std::string text;
        std::string value;
    };

    struct Style {
        glm::vec4 normalColor = glm::vec4(1.0F);
        glm::vec4 hoverColor = glm::vec4(0.8F, 0.8F, 0.8F, 1.0F);
        glm::vec4 pressedColor = glm::vec4(0.6F, 0.6F, 0.6F, 1.0F);
        glm::vec4 arrowColor = glm::vec4(1.0F);
        glm::vec4 textColor = glm::vec4(0.0F, 0.0F, 0.0F, 1.0F);
        glm::vec4 normalOptColor{1.0F, 1.0F, 1.0F, 1.0F};
        glm::vec4 hoverOptColor{0.8F, 0.8F, 0.8F, 1.0F};
        glm::vec4 pressedOptColor{0.6F, 0.6F, 0.6F, 1.0F};
        glm::vec4 highlightOptColor = glm::vec4(0.2F, 0.6F, 1.0F, 1.0F);
    };

    UIDropdown() = default;
    UIDropdown(const UIDropdown& other) = delete;
    UIDropdown& operator=(const UIDropdown& other) = delete;
    UIDropdown(UIDropdown&& other) noexcept = delete;
    UIDropdown& operator=(UIDropdown&& other) noexcept = delete;
    ~UIDropdown() override = default;

    [[nodiscard]] std::string typeName() const override {
        return "UIDropdown";
    }

    void processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                        bool releasedThisFrame, double scrollDelta) override;

    void onValueChanged();

    void setStyle(const Style& style);
    [[nodiscard]] Style getStyle() const {
        return _style;
    };

    void setOptions(const std::vector<Option>& options) {
        _options = options;
        if (_selectedIndex >= _options.size()) {
            _selectedIndex = 0;
        }
    };
    [[nodiscard]] std::vector<Option> getOptions() const {
        return _options;
    };

    void setSelectedIndex(std::size_t index) {
        if (index < _options.size()) {
            _selectedIndex = index;
        }
    };
    [[nodiscard]] std::size_t getSelectedIndex() const {
        return _selectedIndex;
    };

    void toggle();

    void setOptionsContainerGO(GameObject* go);
    [[nodiscard]] GameObject* getOptionsContainerGO() const {
        return _optionsContainerGO;
    }

    void setTriggerGO(GameObject* go);
    [[nodiscard]] GameObject* getTriggerGO() const {
        return _triggerGO;
    }

    void setOptionRender(const OptionRender& optionRender) {
        _optionRender = optionRender;
    }
    [[nodiscard]] OptionRender getOptionRender() const {
        return _optionRender;
    }

    [[nodiscard]] Option getSelectedOption() const;

    void setBackgroundSpriteRenderer(ImageRenderer* spriteRenderer);
    void setArrowSpriteRenderer(ImageRenderer* spriteRenderer);
    void setOptionsBackgroundRenderer(ImageRenderer* spriteRenderer);

    void updateOptionVisuals();
    void applyVisualState();

    void selectOption(std::size_t index);

  private:
    Style _style;
    OptionRender _optionRender;
    std::vector<Option> _options;
    std::size_t _selectedIndex = 0;
    bool _isOpen = false;

    bool _pointerInsideMain = false;

    GameObject* _triggerGO = nullptr;
    GameObject* _optionsContainerGO = nullptr;
    GameObject* _scrollbarGO = nullptr;

    ImageRenderer* _backgroundSpriteRenderer = nullptr;
    ImageRenderer* _arrowSpriteRenderer = nullptr;
    ImageRenderer* _optionsBackgroundRenderer = nullptr;
    std::vector<UIButton*> _optionButtons;
    ImageRenderer* _scrollbarSpriteRenderer = nullptr;
    ImageRenderer* _scrollbarHandleSpriteRenderer = nullptr;
};
} // namespace dzemikk

#endif // DZEMIKK_UIDROPDOWN_H
