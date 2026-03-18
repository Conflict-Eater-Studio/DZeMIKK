#include <cstdio>
#include "core/application.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

dzemikk::Application::Application() {
    spdlog::info("DZeMIKK");
    spdlog::info("spdlog version: {}.{}.{}",
             SPDLOG_VER_MAJOR,
             SPDLOG_VER_MINOR,
             SPDLOG_VER_PATCH);

    if (!glfwInit()) {
        spdlog::critical("Failed to initialize GLFW:");
    }else {
        spdlog::info("GLFW version: {}", glfwGetVersionString());
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "GLFW + GLAD", nullptr, nullptr);

    if (!window) {
        spdlog::critical("Failed to create GLFW window");
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        spdlog::critical("Failed to initialize GLAD");
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    glViewport(0, 0, 800, 600);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

dzemikk::Application::~Application() = default;
