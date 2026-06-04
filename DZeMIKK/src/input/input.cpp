#include "input/input.h"
#include <GLFW/glfw3.h>

namespace dzemikk {

    void Input::initialize() {
    }

    void Input::uninitialize() {
        _windowContext = nullptr;
    }

    void Input::setInputWindow(GLFWwindow* window) {
        _windowContext = window;
    }

    void Input::Update() {
        for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_16; ++jid) {
            if (glfwJoystickIsGamepad(jid)) {
                GLFWgamepadstate state;
                if (glfwGetGamepadState(jid, &state)) {
                    for (int button = 0; button <= GLFW_GAMEPAD_BUTTON_LAST; ++button) {
                        unsigned char current = state.buttons[button];
                        unsigned char previous = _previousGamepadButtons[jid][button];
                        
                        if (current == GLFW_PRESS && previous == GLFW_RELEASE) {
                            dzemikk::GamepadButtonPressedEvent e(jid, button);
                            OnGamepadButtonPressed.Invoke(e);
                        } else if (current == GLFW_RELEASE && previous == GLFW_PRESS) {
                            dzemikk::GamepadButtonReleasedEvent e(jid, button);
                            OnGamepadButtonReleased.Invoke(e);
                        }
                        
                        _previousGamepadButtons[jid][button] = current;
                    }
                }
            }
        }
    }

    void Input::OnEvent(Event& e) {
        if (e.IsInCategory(EventCategoryInput)) {
            dzemikk::EventDispatcher dispatcher(e);

            dispatcher.Dispatch<dzemikk::MouseMovedEvent>([this](dzemikk::MouseMovedEvent& event) {
                OnMouseMoved.Invoke(event);
                return false;
            });

            dispatcher.Dispatch<dzemikk::KeyPressedEvent>([this](dzemikk::KeyPressedEvent& event) {
                OnKeyPressed.Invoke(event);
                return false;
            });

            dispatcher.Dispatch<dzemikk::KeyReleasedEvent>([this](dzemikk::KeyReleasedEvent& event) {
                OnKeyReleased.Invoke(event);
                return false;
            });

            dispatcher.Dispatch<dzemikk::MouseScrolledEvent>([this](dzemikk::MouseScrolledEvent& event) {
                OnMouseScrolled.Invoke(event);
                return false;
            });

            dispatcher.Dispatch<dzemikk::MouseButtonPressedEvent>([this](dzemikk::MouseButtonPressedEvent& event) {
                OnMouseButtonPressed.Invoke(event);
                return false;
            });

            dispatcher.Dispatch<dzemikk::MouseButtonReleasedEvent>([this](dzemikk::MouseButtonReleasedEvent& event) {
                OnMouseButtonReleased.Invoke(event);
                return false;
            });
        }
    }

    bool Input::IsKeyPressed(int keycode) const {
        if (!_windowContext) return false;
        auto state = glfwGetKey(_windowContext, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(int button) const {
        if (!_windowContext) return false;
        auto state = glfwGetMouseButton(_windowContext, button);
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::GetMousePosition() const {
        if (!_windowContext) return {0.0f, 0.0f};
        double xpos, ypos;
        glfwGetCursorPos(_windowContext, &xpos, &ypos);
        return { static_cast<float>(xpos), static_cast<float>(ypos) };
    }

    float Input::GetMouseX() const {
        return GetMousePosition().x;
    }

    float Input::GetMouseY() const {
        return GetMousePosition().y;
    }

    bool Input::IsGamepadButtonPressed(int gamepadId, int button) const {
        GLFWgamepadstate state;
        if (glfwGetGamepadState(gamepadId, &state)) {
            return state.buttons[button] == GLFW_PRESS;
        }
        return false;
    }

    float Input::GetGamepadAxis(int gamepadId, int axis) const {
        GLFWgamepadstate state;
        if (glfwGetGamepadState(gamepadId, &state)) {
            return state.axes[axis];
        }
        return 0.0f;
    }

    bool Input::IsGamepadConnected(int gamepadId) const {
        return glfwJoystickIsGamepad(gamepadId);
    }

}
