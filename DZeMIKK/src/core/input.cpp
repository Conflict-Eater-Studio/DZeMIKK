#include "core/input.h"
#include <GLFW/glfw3.h>

namespace dzemikk {

    GLFWwindow* Input::s_WindowContext = nullptr;

    void Input::Initialize(GLFWwindow* window) {
        s_WindowContext = window;
    }

    bool Input::IsKeyPressed(const KeyCode key) {
        if (!s_WindowContext) return false;
        auto state = glfwGetKey(s_WindowContext, static_cast<int32_t>(key));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(const MouseCode button) {
        if (!s_WindowContext) return false;
        auto state = glfwGetMouseButton(s_WindowContext, static_cast<int32_t>(button));
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::GetMousePosition() {
        if (!s_WindowContext) return {0.0f, 0.0f};
        double xpos, ypos;
        glfwGetCursorPos(s_WindowContext, &xpos, &ypos);
        return { (float)xpos, (float)ypos };
    }

    float Input::GetMouseX() {
        return GetMousePosition().x;
    }

    float Input::GetMouseY() {
        return GetMousePosition().y;
    }
}
