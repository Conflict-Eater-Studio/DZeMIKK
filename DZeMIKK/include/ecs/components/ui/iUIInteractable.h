#ifndef DZEMIKK_IUIINTERACTABLE_H
#define DZEMIKK_IUIINTERACTABLE_H

#include "ecs/component.h"
#include "ecs/components/ui/uiActionRegistry.h"

#include <algorithm>
#include <glm/vec2.hpp>
#include <ranges>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace dzemikk {
class IUIInteractable : public Component {
  public:
    using Base = Component;

    [[nodiscard]] std::string typeName() const override {
        return "IUIInteractable";
    }

    virtual void processPointer(const glm::vec2& point, bool isDown, bool pressedThisFrame,
                                bool releasedThisFrame, double scrollDelta) = 0;

    virtual void onClick() {
        emit(UIEventType::Click);
    }
    virtual void onEnter() {
        emit(UIEventType::Enter);
    }
    virtual void onExit() {
        emit(UIEventType::Exit);
    }

    void addEventListener(UIEventType eventType, const std::string& actionId) {
        if (actionId.empty()) {
            return;
        }

        auto& actionIds = _eventActionIds[eventType];
        if (std::ranges::find(actionIds, actionId) == actionIds.end()) {
            actionIds.emplace_back(actionId);
        }
    }

    void removeEventListener(UIEventType eventType, const std::string& actionId) {
        const auto eventIter = _eventActionIds.find(eventType);
        if (eventIter == _eventActionIds.end()) {
            return;
        }

        auto& actionIds = eventIter->second;
        std::erase(actionIds, actionId);
    }

    void clearEventListeners(UIEventType eventType) {
        _eventActionIds.erase(eventType);
    }

    [[nodiscard]] std::unordered_map<UIEventType, std::vector<std::string>>
    getEventActions() const {
        return _eventActionIds;
    }

  protected:
    void emit(UIEventType eventType,
              std::variant<std::monostate, float, bool> payload = std::monostate{}) {
        UIEvent event(eventType, this, payload);

        const auto actionIdIter = _eventActionIds.find(eventType);
        if (actionIdIter != _eventActionIds.end()) {
            for (const auto& actionId : actionIdIter->second) {
                (void)UIActionRegistry::get().invoke(actionId, event);
            }
        }
    }

    void setPointerDown(bool value) {
        _pointerDown = value;
    }

    void setPointerInside(bool value) {
        _pointerInside = value;
    }

    [[nodiscard]] bool pointerDown() const {
        return _pointerDown;
    }

    [[nodiscard]] bool pointerInside() const {
        return _pointerInside;
    }

    [[nodiscard]] bool pressedInside() const {
        return _pressedInside;
    }

    [[nodiscard]] bool isHovered() const {
        return _hovered;
    }

    void setPressedInside(bool value) {
        _pressedInside = value;
    }

    void updateHoverState() {
        const bool nextHovered = _pointerInside;
        if (nextHovered == _hovered) {
            return;
        }

        _hovered = nextHovered;
        if (_hovered) {
            onEnter();
        } else {
            onExit();
        }
    }

    void processStandardPressRelease(bool pressedThisFrame, bool releasedThisFrame) {
        if (pressedThisFrame) {
            setPressedInside(pointerInside());
        }

        if (releasedThisFrame) {
            if (pressedInside() && pointerInside()) {
                onClick();
            }

            setPressedInside(false);
        }
    }

  private:
    std::unordered_map<UIEventType, std::vector<std::string>> _eventActionIds;

    bool _pointerInside = false;
    bool _pointerDown = false;
    bool _pressedInside = false;
    bool _hovered = false;
};
} // namespace dzemikk

#endif // DZEMIKK_IUIINTERACTABLE_H
