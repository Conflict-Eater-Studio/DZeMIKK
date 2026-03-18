//
// Created by damia on 17.03.2026.
//

#include "core/window.h"

namespace dzemikk {
    Window::Window() {
        GLFWwindow* window = glfwCreateWindow(800, 600, "GLFW + GLAD", nullptr, nullptr);

    }

    Window::Window(int width, int height, const char *title) {
        GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    }

    Window::~Window() = default;
} // dzemikk