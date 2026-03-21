#include "core/window.h"

#include <stdexcept>
#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>


namespace dzemikk {
    Window::Window() : Window(800, 600, "DZeMIKK") {}

    Window::Window(const int width, const int height, const char* title) : window_(nullptr) {
        if (!glfwInit()) {
            spdlog::critical("Failed to initialize GLFW");
            throw std::runtime_error("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!window_) {
            glfwTerminate();
            spdlog::critical("Failed to create GLFW window");
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(window_);

        if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
            glfwTerminate();
            spdlog::critical("Failed to initialize GLAD");
            throw std::runtime_error("Failed to initialize GLAD");
        }

        glViewport(0, 0, width, height);
        spdlog::info("OpenGL initialized successfully");
    }

    Window::~Window() {
        if (window_) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        glfwTerminate();
    }

    bool Window::shouldClose() const {
        return glfwWindowShouldClose(window_);
    }

    void Window::pollEvents() const {
        glfwPollEvents();
    }

    void Window::swapBuffers() const {
        glfwSwapBuffers(window_);
    }

    void Window::clear(const float red, const float green, const float blue, const float alpha) const {
        glClearColor(red, green, blue, alpha);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    GLFWwindow * Window::nativeHandle() const {
        return window_;
    }
}
