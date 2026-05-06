#pragma once

#include "events/event.h"
#include <sstream>

namespace dzemikk {

    class GamepadEvent : public Event {
    public:
        int GetGamepadID() const { return m_GamepadID; }

        EVENT_CLASS_CATEGORY(EventCategoryGamepad | EventCategoryInput)
    protected:
        GamepadEvent(int gamepadID) : m_GamepadID(gamepadID) {}
        int m_GamepadID;
    };

    class GamepadButtonPressedEvent : public GamepadEvent {
    public:
        GamepadButtonPressedEvent(int gamepadID, int button)
            : GamepadEvent(gamepadID), m_Button(button) {}

        int GetButton() const { return m_Button; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "GamepadButtonPressedEvent: Gamepad " << m_GamepadID << " Button " << m_Button;
            return ss.str();
        }

        EVENT_CLASS_TYPE(GamepadButtonPressed)
    private:
        int m_Button;
    };

    class GamepadButtonReleasedEvent : public GamepadEvent {
    public:
        GamepadButtonReleasedEvent(int gamepadID, int button)
            : GamepadEvent(gamepadID), m_Button(button) {}

        int GetButton() const { return m_Button; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "GamepadButtonReleasedEvent: Gamepad " << m_GamepadID << " Button " << m_Button;
            return ss.str();
        }

        EVENT_CLASS_TYPE(GamepadButtonReleased)
    private:
        int m_Button;
    };
}
