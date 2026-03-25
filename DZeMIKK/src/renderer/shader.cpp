#include "renderer/shader.h"

dzemikk::Shader::Shader(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSrc, nullptr);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSrc, nullptr);
    glCompileShader(fragment);

    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

dzemikk::Shader::~Shader() {
    if (program)
        glDeleteProgram(program);
}

void dzemikk::Shader::bind() const {
    glUseProgram(program);
}

void dzemikk::Shader::setMat4(const char* name, const glm::mat4& mat) {
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(mat));
}