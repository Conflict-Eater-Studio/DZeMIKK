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
void UIDropdown::processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                                bool releasedThisFrame, double scrollDelta) {
    setPointerDown(isDown);

    const bool pressedInsideMain = _pointerInsideMain && isDown;

    setPointerInside(_pointerInsideMain);
    updateHoverState();
    processStandardPressRelease(pressedThisFrame, releasedThisFrame);

    if (pressedInsideMain && _backgroundSpriteRenderer->getRectTransform()->containsPoint(point)) {
        _isOpen = !_isOpen;
        if (_optionsContainerGO) {
            _optionsContainerGO->enabled(_isOpen);
        }
        onClick();
    }

    applyVisualState();
}

void UIDropdown::updateOptionVisuals() {
    if (_optionsContainerGO == nullptr) {
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

void UIDropdown::setArrowSpriteRenderer(ImageRenderer* spriteRenderer) {
    _arrowSpriteRenderer = spriteRenderer;
    if (_arrowSpriteRenderer) {
        _arrowSpriteRenderer->setColor(_style.arrowColor);
    }
}

void UIDropdown::setBackgroundSpriteRenderer(ImageRenderer* spriteRenderer) {
    _backgroundSpriteRenderer = spriteRenderer;
    if (_backgroundSpriteRenderer) {
        _backgroundSpriteRenderer->setColor(_style.normalColor);
    }
}

void UIDropdown::setOptionsBackgroundRenderer(ImageRenderer* spriteRenderer) {
    _optionsBackgroundRenderer = spriteRenderer;
    if (_optionsBackgroundRenderer) {
        _optionsBackgroundRenderer->setColor(_style.normalColor);
    }
}

void UIDropdown::applyVisualState() {
    if (_backgroundSpriteRenderer) {
        if (pointerInside() && pointerDown()) {
            _backgroundSpriteRenderer->setColor(_style.pressedColor);
        } else if (isHovered()) {
            _backgroundSpriteRenderer->setColor(_style.hoverColor);
        } else {
            _backgroundSpriteRenderer->setColor(_style.normalColor);
        }
    }

    if (_triggerGO) {
        auto* textRenderer =
            _triggerGO->getComponent<UIButton>()->getTextGO()->getComponent<UITextRenderer>();
        if (textRenderer) {
            textRenderer->text = getSelectedOption().text;
        }
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

void UIDropdown::setOptionsContainerGO(GameObject* go) {
    _optionsContainerGO = go;
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

void UIDropdown::setTriggerGO(GameObject* go) {
    _triggerGO = go;
}
} // namespace dzemikk
