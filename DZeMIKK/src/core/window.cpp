//
// Created by damia on 17.03.2026.
//

#include "core/window.h"
#include "spdlog/spdlog.h"

namespace dzemikk {
    Window::Window() : Window(600, 800, "DZeMIKK") {}

    Window::Window(int width, int height, const char *title) {
        if (!glfwInit()) {
            spdlog::critical("Failed to initialize GLFW:");
        }else {
            spdlog::info("GLFW version: {}", GLFW_CONTEXT_VERSION_MAJOR);
        }
        glfwCreateWindow(width, height, title, nullptr, nullptr);
        this->window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    }

    Window::~Window() = default;
} // dzemikk