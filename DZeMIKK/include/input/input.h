#ifndef DZEMIKK_INPUT_H
#define DZEMIKK_INPUT_H

#include "core/iEngineModule.h"
#include "events/event.h"
#include "events/mouse_event.h"
#include "events/key_event.h"
#include "events/gamepad_event.h"
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <algorithm>

struct GLFWwindow;

namespace dzemikk {

    using ListenerID = uint32_t;

    template <typename T>
    class Action {
    public:
        ListenerID addListener(std::function<void(T&)> listener) {
            ListenerID id = ++m_nextId;
            m_listeners.push_back({id, listener});
            return id;
        }

        void removeListener(ListenerID id) {
            m_listeners.erase(
                std::remove_if(m_listeners.begin(), m_listeners.end(), 
                    [id](const auto& pair) { return pair.first == id; }),
                m_listeners.end()
            );
        }

        void Invoke(T& e) {
            for (auto& l : m_listeners) l.second(e);
        }
    private:
        ListenerID m_nextId = 0;
        std::vector<std::pair<ListenerID, std::function<void(T&)>>> m_listeners;
    };

    class Input : public IEngineModule {
    public:
        Input() = default;
        ~Input() override = default;

        void initialize() override;
        void uninitialize() override;

        void setInputWindow(GLFWwindow* window);
        
        void Update() override;
        void OnEvent(Event& e);

        Action<dzemikk::MouseMovedEvent> OnMouseMoved;
        Action<dzemikk::MouseScrolledEvent> OnMouseScrolled;
        Action<dzemikk::MouseButtonPressedEvent> OnMouseButtonPressed;
        Action<dzemikk::MouseButtonReleasedEvent> OnMouseButtonReleased;
        Action<dzemikk::KeyPressedEvent> OnKeyPressed;
        Action<dzemikk::KeyReleasedEvent> OnKeyReleased;

        Action<dzemikk::GamepadButtonPressedEvent> OnGamepadButtonPressed;
        Action<dzemikk::GamepadButtonReleasedEvent> OnGamepadButtonReleased;


        ListenerID BindKeyPressed(int keycode, std::function<void()> callback) {
            return OnKeyPressed.addListener([keycode, callback](dzemikk::KeyPressedEvent& e) {
                if (e.GetKeyCode() == keycode && e.GetRepeatCount() == 0) callback();
            });
        }

        ListenerID BindKeyReleased(int keycode, std::function<void()> callback) {
            return OnKeyReleased.addListener([keycode, callback](dzemikk::KeyReleasedEvent& e) {
                if (e.GetKeyCode() == keycode) callback();
            });
        }

        ListenerID BindGamepadButtonPressed(int button, std::function<void()> callback) {
            return OnGamepadButtonPressed.addListener([button, callback](dzemikk::GamepadButtonPressedEvent& e) {
                if (e.GetButton() == button) callback();
            });
        }

        template <typename T>
        ListenerID BindKeyPressed(int keycode, T* instance, void (T::*memberFunction)()) {
            return OnKeyPressed.addListener([keycode, instance, memberFunction](dzemikk::KeyPressedEvent& e) {
                if (e.GetKeyCode() == keycode && e.GetRepeatCount() == 0) (instance->*memberFunction)();
            });
        }

        template <typename T>
        ListenerID BindKeyReleased(int keycode, T* instance, void (T::*memberFunction)()) {
            return OnKeyReleased.addListener([keycode, instance, memberFunction](dzemikk::KeyReleasedEvent& e) {
                if (e.GetKeyCode() == keycode) (instance->*memberFunction)();
            });
        }

        template <typename T>
        ListenerID BindGamepadButtonPressed(int button, T* instance, void (T::*memberFunction)()) {
            return OnGamepadButtonPressed.addListener([button, instance, memberFunction](dzemikk::GamepadButtonPressedEvent& e) {
                if (e.GetButton() == button) (instance->*memberFunction)();
            });
        }

        bool IsKeyPressed(int keycode) const;
        bool IsMouseButtonPressed(int button) const;
        glm::vec2 GetMousePosition() const;
        float GetMouseX() const;
        float GetMouseY() const;

        bool IsGamepadButtonPressed(int gamepadId, int button) const;
        float GetGamepadAxis(int gamepadId, int axis) const;
        bool IsGamepadConnected(int gamepadId) const;

    private:
        GLFWwindow* _windowContext = nullptr;
        unsigned char _previousGamepadButtons[16][15] = {0}; 
    };

}

#endif // DZEMIKK_INPUT_H
