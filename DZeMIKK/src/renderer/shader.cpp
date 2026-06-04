#include <iostream>

#include "renderer/shader.h"

dzemikk::Shader::Shader(const char* vertexSrc, const char* fragmentSrc) {
    _vertSrc = vertexSrc;
    _fragSrc = fragmentSrc;
}

dzemikk::Shader::~Shader() {
    if (_program)
        glDeleteProgram(_program);
}

void dzemikk::Shader::bind() const {
    glUseProgram(_program);
}

void dzemikk::Shader::unbind() const {
    glUseProgram(0);
}

void dzemikk::Shader::uploadToGPU() {
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    const char* v = _vertSrc.c_str();
    glShaderSource(vertex, 1, &v, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    const char* f = _fragSrc.c_str();
    glShaderSource(fragment, 1, &f, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    _program = glCreateProgram();
    glAttachShader(_program, vertex);
    glAttachShader(_program, fragment);
    glLinkProgram(_program);
    checkCompileErrors(_program, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    unsigned int uniformBlockIndex = glGetUniformBlockIndex(_program, "Matrices");
    glUniformBlockBinding(_program, uniformBlockIndex, 0);
}

void dzemikk::Shader::setFloat(const char* name, float value) {
    glUniform1f(glGetUniformLocation(_program, name), value);
}

void dzemikk::Shader::setInt(const char* name, int value) {
    glUniform1i(glGetUniformLocation(_program, name), value);
}

void dzemikk::Shader::setBool(const char* name, bool value) {
    glUniform1i(glGetUniformLocation(_program, name), value ? 1 : 0);
}

void dzemikk::Shader::setVec2(const char* name, const glm::vec2& vec) {
    glUniform2fv(glGetUniformLocation(_program, name), 1, glm::value_ptr(vec));
}

void dzemikk::Shader::setVec3(const char* name, const glm::vec3& vec) {
    glUniform3fv(glGetUniformLocation(_program, name), 1, glm::value_ptr(vec));
}

void dzemikk::Shader::setVec4(const char* name, const glm::vec4& vec) {
    glUniform4fv(glGetUniformLocation(_program, name), 1, glm::value_ptr(vec));
}

void dzemikk::Shader::setIVec2(const char* name, const glm::ivec2& vec) {
    glUniform2iv(glGetUniformLocation(_program, name), 1, glm::value_ptr(vec));
}

void dzemikk::Shader::setIVec3(const char* name, const glm::ivec3& vec) {
    glUniform3iv(glGetUniformLocation(_program, name), 1, glm::value_ptr(vec));
}

void dzemikk::Shader::setIVec4(const char* name, const glm::ivec4& vec) {
    glUniform4iv(glGetUniformLocation(_program, name), 1, glm::value_ptr(vec));
}

void dzemikk::Shader::setBVec2(const char* name, const glm::bvec2& vec) {
    GLint vals[2] = {vec.x ? 1 : 0, vec.y ? 1 : 0};
    glUniform2iv(glGetUniformLocation(_program, name), 1, vals);
}

void dzemikk::Shader::setBVec3(const char* name, const glm::bvec3& vec) {
    GLint vals[3] = {vec.x ? 1 : 0, vec.y ? 1 : 0, vec.z ? 1 : 0};
    glUniform3iv(glGetUniformLocation(_program, name), 1, vals);
}

void dzemikk::Shader::setBVec4(const char* name, const glm::bvec4& vec) {
    GLint vals[4] = {vec.x ? 1 : 0, vec.y ? 1 : 0, vec.z ? 1 : 0, vec.w ? 1 : 0};
    glUniform4iv(glGetUniformLocation(_program, name), 1, vals);
}

void dzemikk::Shader::setMat2(const char* name, const glm::mat2& mat) {
    glUniformMatrix2fv(glGetUniformLocation(_program, name), 1, GL_FALSE,
                       glm::value_ptr(mat));
}

void dzemikk::Shader::setMat3(const char* name, const glm::mat3& mat) {
    glUniformMatrix3fv(glGetUniformLocation(_program, name), 1, GL_FALSE,
                       glm::value_ptr(mat));
}

void dzemikk::Shader::setMat4(const char* name, const glm::mat4& mat) {
    glUniformMatrix4fv(glGetUniformLocation(_program, name), 1, GL_FALSE,
                       glm::value_ptr(mat));
}

void dzemikk::Shader::setMat4Array(const char* name,
                                   const std::vector<glm::mat4>& matrices) const {
    if (matrices.empty())
        return;

    GLint location = glGetUniformLocation(_program, name);
    if (location == -1)
        return;

    glUniformMatrix4fv(location, static_cast<GLsizei>(matrices.size()), GL_FALSE,
                       reinterpret_cast<const float*>(matrices.data()));
}

void dzemikk::Shader::setSampler(const char* name, int textureUnit) {
    glUniform1i(glGetUniformLocation(_program, name), textureUnit);
}

void dzemikk::Shader::checkCompileErrors(GLuint shader, const char* type) const {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "[Shader] Compilation error (" << type << "): " << infoLog << "\n";
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "[Shader] Linking error: " << infoLog << "\n";
        }
    }
}

dzemikk::Shader::Shader(Shader&& other) noexcept {
    _program = other._program;

    other._program = 0;
}

dzemikk::Shader& dzemikk::Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (_program)
            glDeleteProgram(_program); 

        _program = other._program;

        other._program = 0; 
    }
    return *this;
}

const GLuint dzemikk::Shader::getProgramID() const {
    return _program;
}

void dzemikk::Shader::recompile(const char* vertSrc, const char* fragSrc) {
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertSrc, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    GLint success;
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glDeleteShader(vertex);
        return;
    }

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragSrc, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return;
    }

    GLuint newProgram = glCreateProgram();
    glAttachShader(newProgram, vertex);
    glAttachShader(newProgram, fragment);
    glLinkProgram(newProgram);
    checkCompileErrors(newProgram, "PROGRAM");

    glGetProgramiv(newProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(newProgram);
        return;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    unsigned int uniformBlockIndex = glGetUniformBlockIndex(newProgram, "Matrices");
    if (uniformBlockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(newProgram, uniformBlockIndex, 0);
    }

    if (_program) {
        glDeleteProgram(_program);
    }

    _program = newProgram;
}