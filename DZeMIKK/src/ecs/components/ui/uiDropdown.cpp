#include "ecs/components/ui/uiDropdown.h"

#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/uiActionRegistry.h"
#include "ecs/components/ui/uiBuilder.h"
#include "ecs/components/ui/uiEvent.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <boost/uuid.hpp>

namespace dzemikk {
namespace {
bool hasSelection(const UIDropdown& dropdown) {
    return dropdown.getSelectedIndex() != UIDropdown::noSelection();
}
} // namespace

UIDropdown::UIDropdown() : _triggerActionId(boost::uuids::random_generator()()) {
    UIActionRegistry::get().registerAction([&](const UIEvent& event) { toggle(); },
                                           boost::uuids::to_string(_triggerActionId));
};

void UIDropdown::processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                                bool releasedThisFrame, double scrollDelta) {
    setPointerDown(isDown);

    const bool pressedInsideMain = _pointerInsideMain && isDown;

    setPointerInside(_pointerInsideMain);
    updateHoverState();
    processStandardPressRelease(pressedThisFrame, releasedThisFrame);

    applyVisualState();
}

void UIDropdown::setTriggerActionId(const boost::uuids::uuid& actionId) {
    if (_triggerActionId == actionId) {
        return;
    }

    UIActionRegistry::get().renameAction(boost::uuids::to_string(_triggerActionId),
                                         boost::uuids::to_string(actionId));
    _triggerActionId = actionId;
}

void UIDropdown::updateOptionVisuals() {
    if (getOptionsContainerGO() == nullptr) {
        return;
    }

    auto& reg = UIActionRegistry::get();
    for (const auto& actionId : _optionActionIds) {
        reg.unregisterAction(actionId);
    }
    _optionActionIds.clear();
    _optionButtons.clear();

    _optionsContainerGO->destroyChildren();
    _optionActionIds.reserve(_options.size());
    _optionButtons.reserve(_options.size());

    for (std::size_t i = 0; i < _options.size(); i++) {
        auto* optBtnGo = UIBuilder::createButton(
            _optionsContainerGO,
            {
                .name = _owner->getName() + "_Option_" + std::to_string(i),
                .position = {0.0F, -static_cast<float>(i) * _optionRender.height},
                .size = {_optionsContainerGO->rectTransform()->getSize()[0], _optionRender.height},
                .anchorMin = {0.0F, 1.0F},
                .anchorMax = {0.0F, 1.0F},
                .pivot = {0.0F, 1.0F},
                .text = _options[i].text,
                .textFont = _optionRender.font,
                .textVAlign = _optionRender.textVAlign,
                .textHAlign = _optionRender.textHAlign,
                .normalColor =
                    hasSelection(*this) && i == _selectedIndex ? _style.highlightOptColor
                                                                : _style.normalOptColor,
                .hoverColor = _style.hoverOptColor,
                .pressedColor = _style.pressedOptColor,
                .mesh = _optionRender.mesh,
                .material = _optionRender.material,
            });
        auto* btn = optBtnGo->getComponent<UIButton>();
        auto actionId = boost::uuids::to_string(boost::uuids::random_generator()());
        reg.registerAction([this, i](const UIEvent& event) { selectOption(i); }, actionId);
        btn->addEventListener(UIEventType::Click, actionId);
        _optionActionIds.push_back(actionId);
        _optionButtons.push_back(btn);
    }
}

void UIDropdown::selectOption(std::size_t index) {
    if (index >= _options.size() || index == _selectedIndex) {
        return;
    }

    _selectedIndex = index;
    applyOptionButtonColors();
    toggle();

    applyVisualState();
    onValueChanged();
}

void UIDropdown::applyOptionButtonColors() {
    for (std::size_t i = 0; i < _optionButtons.size(); i++) {
        auto* btn = _optionButtons[i];
        if (btn == nullptr) {
            continue;
        }

        auto style = btn->getStyle();
        style.normalColor = hasSelection(*this) && i == _selectedIndex ? _style.highlightOptColor
                                                                        : _style.normalOptColor;
        btn->setStyle(style);
    }
}

void UIDropdown::onValueChanged() {
    emit(UIEventType::ValueChanged);
}

void UIDropdown::applyVisualState() {
    auto* textRenderer =
        _owner->getComponent<UIButton>()->getTextGO()->getComponent<UITextRenderer>();
    if (textRenderer) {
        textRenderer->text = hasSelection(*this) ? getSelectedOption().text : "";
    }
}

void UIDropdown::toggle() {
    _isOpen = !_isOpen;
    if (getOptionsContainerGO() != nullptr) {
        if (_isOpen) {
            updateOptionVisuals();
        }
        _optionsContainerGO->enabled(_isOpen);
    }
}

UIDropdown::OptionRender UIDropdown::getOptionRender() const {
    return _optionRender;
}

void UIDropdown::setStyle(const Style& style) {
    _style = style;
    applyVisualState();
}

UIDropdown::Option UIDropdown::getSelectedOption() const {
    if (_selectedIndex < _options.size()) {
        return _options[_selectedIndex];
    }
    return {};
}

GameObject* UIDropdown::getOptionsContainerGO() {
    if (_optionsContainerGO == nullptr) {
        for (auto* child : _owner->getChildren()) {
            if (child->getName().find("_OptionsBG") != std::string::npos) {
                _optionsContainerGO = child;
                break;
            }
        }
    }

    return _optionsContainerGO;
}

ImageRenderer* UIDropdown::getOptionsBackgroundRenderer() {
    if (getOptionsContainerGO() == nullptr) {
        return nullptr;
    }

    if (_optionsBackgroundRenderer == nullptr) {
        _optionsBackgroundRenderer = _optionsContainerGO->getComponent<ImageRenderer>();
    }

    return _optionsBackgroundRenderer;
}

void UIDropdown::init(const Style& style, const OptionRender& render,
                      const std::vector<Option>& options, std::size_t selectedIndex,
                      const boost::uuids::uuid& triggerActionId) {
    _style = style;
    _optionRender = render;
    _options = options;
    _selectedIndex = selectedIndex < _options.size() ? selectedIndex : noSelection();
    setTriggerActionId(triggerActionId);
}
} // namespace dzemikk
