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

}
