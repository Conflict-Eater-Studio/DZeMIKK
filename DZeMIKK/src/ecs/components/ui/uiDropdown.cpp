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
UIDropdown::UIDropdown() {
    UIActionRegistry::get().registerAction(
        [&](const UIEvent& event) { toggle(); },
        boost::uuids::to_string(boost::uuids::random_generator()()));
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

void UIDropdown::updateOptionVisuals() {
    if (getOptionsContainerGO() == nullptr) {
        return;
    }

    auto* scene = _owner->getScene();

    _optionsContainerGO->destroyChildren();

    auto& reg = UIActionRegistry::get();
    for (auto* btn : _optionButtons) {
        for (const auto& [key, value] : btn->getEventActions()) {
            for (const auto& actionId : value) {
                reg.unregisterAction(actionId);
            }
        }
    }
    _optionButtons.clear();
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
                .normalColor = _style.normalOptColor,
                .hoverColor = _style.hoverOptColor,
                .pressedColor = _style.pressedOptColor,
                .mesh = _optionRender.mesh,
                .material = _optionRender.material,
            });
        auto* btn = optBtnGo->getComponent<UIButton>();
        _optionButtons.push_back(btn);
        auto actionId = boost::uuids::to_string(boost::uuids::random_generator()());
        UIActionRegistry::get().registerAction([this, i](const UIEvent& event) { selectOption(i); },
                                               actionId);
        btn->addEventListener(UIEventType::Click, actionId);

        _optionsContainerGO->enabled(_isOpen);
    }
}

void UIDropdown::selectOption(std::size_t index) {
    if (index >= _options.size() || index == _selectedIndex) {
        return;
    }

    _selectedIndex = index;

    applyVisualState();
    onValueChanged();
}

void UIDropdown::onValueChanged() {
    emit(UIEventType::ValueChanged);
}

void UIDropdown::applyVisualState() {
    auto* textRenderer =
        _owner->getComponent<UIButton>()->getTextGO()->getComponent<UITextRenderer>();
    if (textRenderer) {
        textRenderer->text = getSelectedOption().text;
    }

    if (!_optionButtons.empty()) {
        for (std::size_t i = 0; i < _optionButtons.size(); i++) {
            if (i == _selectedIndex) {
                _optionButtons[i]->setStyle({
                    .normalColor = _style.highlightOptColor,
                    .hoverColor = _style.hoverOptColor,
                    .pressedColor = _style.pressedOptColor,
                });
            } else {
                _optionButtons[i]->setStyle({
                    .normalColor = _style.normalOptColor,
                    .hoverColor = _style.hoverOptColor,
                    .pressedColor = _style.pressedOptColor,
                });
            }
        }
    }
}

void UIDropdown::toggle() {
    _isOpen = !_isOpen;
    if (_optionsContainerGO) {
        _optionsContainerGO->enabled(_isOpen);
    }
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
} // namespace dzemikk
