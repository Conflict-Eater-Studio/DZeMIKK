#include <iostream>

#include "renderer/shader.h"

dzemikk::Shader::Shader(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSrc, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSrc, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    _program = glCreateProgram();
    glAttachShader(_program, vertex);
    glAttachShader(_program, fragment);
    glLinkProgram(_program);
    checkCompileErrors(_program, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
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

GLint dzemikk::Shader::getUniformLocation(const std::string& name) const {
    if (_uniformLocationCache.find(name) != _uniformLocationCache.end())
        return _uniformLocationCache[name];
    GLint location = glGetUniformLocation(_program, name.c_str());
    if (location == -1)
        std::cerr << "[Shader] Warning: uniform '" << name << "' doesn't exist!\n";
    _uniformLocationCache[name] = location;
    return location;
}

void dzemikk::Shader::setFloat(const std::string& name, float value) {
    glUniform1f(getUniformLocation(name), value);
}

void dzemikk::Shader::setInt(const std::string& name, int value) {
    glUniform1i(getUniformLocation(name), value);
}

void dzemikk::Shader::setBool(const std::string& name, bool value) {
    glUniform1i(getUniformLocation(name), value ? 1 : 0);
}

void dzemikk::Shader::setVec2(const std::string& name, const glm::vec2& vec) {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(vec));
}

void dzemikk::Shader::setVec3(const std::string& name, const glm::vec3& vec) {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(vec));
}

void dzemikk::Shader::setVec4(const std::string& name, const glm::vec4& vec) {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(vec));
}

void dzemikk::Shader::setIVec2(const std::string& name, const glm::ivec2& vec) {
    glUniform2iv(getUniformLocation(name), 1, glm::value_ptr(vec));
}

void dzemikk::Shader::setIVec3(const std::string& name, const glm::ivec3& vec) {
    glUniform3iv(getUniformLocation(name), 1, glm::value_ptr(vec));
}

void dzemikk::Shader::setIVec4(const std::string& name, const glm::ivec4& vec) {
    glUniform4iv(getUniformLocation(name), 1, glm::value_ptr(vec));
}

void dzemikk::Shader::setBVec2(const std::string& name, const glm::bvec2& vec) {
    GLint vals[2] = {vec.x ? 1 : 0, vec.y ? 1 : 0};
    glUniform2iv(getUniformLocation(name), 1, vals);
}

void dzemikk::Shader::setBVec3(const std::string& name, const glm::bvec3& vec) {
    GLint vals[3] = {vec.x ? 1 : 0, vec.y ? 1 : 0, vec.z ? 1 : 0};
    glUniform3iv(getUniformLocation(name), 1, vals);
}

void dzemikk::Shader::setBVec4(const std::string& name, const glm::bvec4& vec) {
    GLint vals[4] = {vec.x ? 1 : 0, vec.y ? 1 : 0, vec.z ? 1 : 0, vec.w ? 1 : 0};
    glUniform4iv(getUniformLocation(name), 1, vals);
}

void dzemikk::Shader::setMat2(const std::string& name, const glm::mat2& mat) {
    glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void dzemikk::Shader::setMat3(const std::string& name, const glm::mat3& mat) {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void dzemikk::Shader::setMat4(const std::string& name, const glm::mat4& mat) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void dzemikk::Shader::setSampler(const std::string& name, int textureUnit) {
    glUniform1i(getUniformLocation(name), textureUnit);
}

void dzemikk::Shader::checkCompileErrors(GLuint shader, const std::string& type) const {
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
    _uniformLocationCache = std::move(other._uniformLocationCache);

    other._program = 0;
}

dzemikk::Shader& dzemikk::Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (_program)
            glDeleteProgram(_program); 

        _program = other._program;
        _uniformLocationCache = std::move(other._uniformLocationCache);

        other._program = 0; 
    }
    return *this;
}

const GLuint dzemikk::Shader::getProgramID() const {
    return _program;
}