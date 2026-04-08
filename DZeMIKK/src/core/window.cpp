#include "core/window.h"

#include <glad/glad.h>
#include <stdexcept>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

namespace dzemikk {
    Window::Window() : Window(1920, 1080, "DZeMIKK") {}

    Window::Window(const int width, const int height, const char* title) : window_(nullptr) {
        if (!glfwInit()) {
#if DZEMIKK_DEV_TOOLS
            spdlog::critical("Failed to initialize GLFW");
#endif

            throw std::runtime_error("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);

        window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!window_) {
            glfwTerminate();
#if DZEMIKK_DEV_TOOLS
            spdlog::critical("Failed to create GLFW window");
#endif

            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(window_);

        if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
            glfwTerminate();
#if DZEMIKK_DEV_TOOLS
            spdlog::critical("Failed to initialize GLAD");
#endif
            throw std::runtime_error("Failed to initialize GLAD");
        }

        glViewport(0, 0, width, height);
#if DZEMIKK_DEV_TOOLS
        spdlog::info("OpenGL initialized successfully");
#endif
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

    GLFWwindow* Window::nativeHandle() const {
        return window_;
    }
    void Window::Initialize() {

    }
    void Window::UnInitialize() {

        }
    } // namespace dzemikk
