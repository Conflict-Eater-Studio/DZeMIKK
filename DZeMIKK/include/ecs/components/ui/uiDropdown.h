#ifndef DZEMIKK_UIDROPDOWN_H
#define DZEMIKK_UIDROPDOWN_H

#include "ecs/components/ui/iUIInteractable.h"
#include "ecs/components/ui/uiButton.h"
#include "ecs/components/ui/uiTextRenderer.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <limits>
#include <vector>

namespace dzemikk {
class AssetManager;
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

    UIDropdown();
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
        if (_selectedIndex != noSelection() && _selectedIndex >= _options.size()) {
            _selectedIndex = noSelection();
        }
    };
    void setOptionRender(const OptionRender& render) {
        _optionRender = render;
    };
    [[nodiscard]] std::vector<Option> getOptions() const {
        return _options;
    };

    void setSelectedIndex(std::size_t index) {
        if (index == noSelection()) {
            _selectedIndex = noSelection();
        } else if (index < _options.size()) {
            _selectedIndex = index;
        }
    };
    [[nodiscard]] std::size_t getSelectedIndex() const {
        return _selectedIndex;
    };

    [[nodiscard]] static constexpr std::size_t noSelection() {
        return std::numeric_limits<std::size_t>::max();
    }

    void setTriggerActionId(const boost::uuids::uuid& actionId);
    [[nodiscard]] boost::uuids::uuid getTriggerActionId() const {
        return _triggerActionId;
    }

    void toggle();

    [[nodiscard]] GameObject* getOptionsContainerGO();

    [[nodiscard]] OptionRender getOptionRender() const;
    [[nodiscard]] Option getSelectedOption() const;

    // [[nodiscard]] ImageRenderer* getArrowSpriteRenderer();
    [[nodiscard]] ImageRenderer* getOptionsBackgroundRenderer();
    // [[nodiscard]] ImageRenderer* getScrollbarSpriteRenderer();
    // [[nodiscard]] ImageRenderer* getScrollbarHandleSpriteRenderer();

    void updateOptionVisuals();
    void applyVisualState();
    void applyOptionButtonColors();

    void selectOption(std::size_t index);

    void init(const Style& style, const OptionRender& render, const std::vector<Option>& options,
              std::size_t selectedIndex, const boost::uuids::uuid& triggerActionId);

  private:
    boost::uuids::uuid _triggerActionId;

    Style _style;
    OptionRender _optionRender;
    std::vector<Option> _options;
    std::size_t _selectedIndex = noSelection();
    bool _isOpen = false;

    bool _pointerInsideMain = false;

    GameObject* _optionsContainerGO = nullptr;
    GameObject* _scrollbarGO = nullptr;

    ImageRenderer* _arrowSpriteRenderer = nullptr;
    ImageRenderer* _optionsBackgroundRenderer = nullptr;
    ImageRenderer* _scrollbarSpriteRenderer = nullptr;
    ImageRenderer* _scrollbarHandleSpriteRenderer = nullptr;

    std::vector<std::string> _optionActionIds;
    std::vector<UIButton*> _optionButtons;
};
} // namespace dzemikk

#endif // DZEMIKK_UIDROPDOWN_H
