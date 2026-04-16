#include "core/window.h"

#include <glad/glad.h>
#include <stdexcept>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

#include "events/application_event.h"
#include "events/key_event.h"
#include "events/mouse_event.h"

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

        data_.Title = title;
        data_.Width = width;
        data_.Height = height;

        glfwMakeContextCurrent(window_);
        glfwSetWindowUserPointer(window_, &data_);

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

        // GLFW Callbacks
        glfwSetWindowSizeCallback(window_, [](GLFWwindow* window, int width, int height) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.Width = width;
            data.Height = height;

            WindowResizeEvent event(width, height);
            data.EventCallback(event);
        });

        glfwSetWindowCloseCallback(window_, [](GLFWwindow* window) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            WindowCloseEvent event;
            data.EventCallback(event);
        });

        glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event(key, 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    KeyReleasedEvent event(key);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT: {
                    KeyPressedEvent event(key, 1);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action) {
                case GLFW_PRESS: {
                    MouseButtonPressedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(window_, [](GLFWwindow* window, double xOffset, double yOffset) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            data.EventCallback(event);
        });

        glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xPos, double yPos) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseMovedEvent event((float)xPos, (float)yPos);
            data.EventCallback(event);
        });
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
    void Window::initialize() {

    }
    void Window::uninitialize() {

        }
    } // namespace dzemikk
