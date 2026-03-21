#include "renderer/renderer.h"

#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <iostream>

namespace dzemikk {

Renderer::Renderer() {
    initCube();
    initCubeShader();

    initRectangle();
    initRectShader();
}

Renderer::~Renderer() {
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteProgram(cubeShader);

    glDeleteVertexArrays(1, &rectVAO);
    glDeleteBuffers(1, &rectVBO);
    glDeleteProgram(rectShader);
}

void Renderer::initCube() {
    float vertices[] = {// FRONT
                        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
                        -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f,

                        // BACK
                        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
                        -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f,

                        // LEFT
                        -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
                        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,

                        // RIGHT
                        0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f,
                        0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,

                        // TOP
                        -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
                        -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f,

                        // BOTTOM
                        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
                        0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f};

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Renderer::initCubeShader() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.5, 0.2, 1.0);
        }
    )";

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex);

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);

    cubeShader = glCreateProgram();
    glAttachShader(cubeShader, vertex);
    glAttachShader(cubeShader, fragment);
    glLinkProgram(cubeShader);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Renderer::initRectangle() {
    float vertices[] = {
                        -0.5f, -0.5f, 0.0f, 0.5f,  -0.5f, 0.0f, 0.5f,  0.5f,  0.0f,
                        0.5f,  0.5f,  0.0f, -0.5f, 0.5f,  0.0f, -0.5f, -0.5f, 0.0f};

    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);

    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Renderer::initRectShader() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 model;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * model * vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(0.2, 0.8, 0.3, 1.0);
        }
    )";

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex);

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);

    rectShader = glCreateProgram();
    glAttachShader(rectShader, vertex);
    glAttachShader(rectShader, fragment);
    glLinkProgram(rectShader);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Renderer::DrawCube() {
    glUseProgram(cubeShader);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.0f));
    glm::mat4 view = glm::lookAt(glm::vec3(1.5f, 1.5f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(cubeShader, "model"), 1, GL_FALSE,
                       glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(cubeShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(cubeShader, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Renderer::DrawRectangle() {
    glUseProgram(rectShader);

    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(-0.5f, -0.5f, 0.0f));

    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 1.0f));

    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    glUniformMatrix4fv(glGetUniformLocation(rectShader, "model"), 1, GL_FALSE,
                       glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(rectShader, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));

    glBindVertexArray(rectVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);
}

} 