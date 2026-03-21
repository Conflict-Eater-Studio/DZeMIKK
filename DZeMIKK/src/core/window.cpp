#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include "core/window.h"

namespace dzemikk {
    Window::Window() : Window(600, 800, "DZeMIKK") {}

    Window::Window(int width, int height, const char *title) {
        if (!glfwInit()) {
            spdlog::critical("Failed to initialize GLFW:");
        }else {
            spdlog::info("GLFW version: {}", GLFW_CONTEXT_VERSION_MAJOR);
        }
        this->window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);

        glfwMakeContextCurrent(window_);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            spdlog::critical("Failed to initialize GLAD");
            glfwDestroyWindow(window_);
            glfwTerminate();
        }

        glViewport(0, 0, 800, 600);

        while (!glfwWindowShouldClose(window_)) {
            glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glfwSwapBuffers(window_);
            glfwPollEvents();
        }
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
    Window::~Window() = default;
}
