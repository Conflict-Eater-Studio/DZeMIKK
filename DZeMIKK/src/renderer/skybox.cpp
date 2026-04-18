#include "renderer/skybox.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "stb/stb_image.h"

dzemikk::Skybox::Skybox() {
    initCube();
}

void dzemikk::Skybox::initCube() {
    float rawVertices[] = {// cube positions
                        -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                        1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                        -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                        -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                        1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                        -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                        1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                        -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                        1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                        1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    std::vector<StaticVertex> vertices;
    vertices.reserve(36);

    for (int i = 0; i < 36; i++) {
        StaticVertex v{};
        v.position = {rawVertices[i * 3 + 0], rawVertices[i * 3 + 1], rawVertices[i * 3 + 2]};

        v.normal = glm::vec3(0.0f); 
        vertices.push_back(v);
    }

    std::vector<unsigned int> indices;

    _cubeMesh = std::make_unique<StaticMesh>();
    _cubeMesh->create(vertices, indices);
}

dzemikk::Skybox::~Skybox() {
    if (_cubemapTex != 0) {
        glDeleteTextures(1, &_cubemapTex);
        _cubemapTex = 0;
    }
}

void dzemikk::Skybox::loadCubemap(const std::vector<std::string>& faces) {
    if (faces.size() != 6)
        throw std::runtime_error("Skybox requires exactly 6 textures");

    stbi_set_flip_vertically_on_load(false);

    unsigned int newTex;
    glGenTextures(1, &newTex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, newTex);

    int width, height, channels;

    for (size_t i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);

        if (!data) {
            glDeleteTextures(1, &newTex); 
            throw std::runtime_error("Failed to load cubemap: " + faces[i]);
        }

        GLenum format = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_RED;

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if (_cubemapTex != 0)
        glDeleteTextures(1, &_cubemapTex);

    _cubemapTex = newTex;

    _mode = Mode::Cubemap;
}

void dzemikk::Skybox::setColor(const glm::vec3& color) {
    _color = color;
    _mode = Mode::Color;
}

void dzemikk::Skybox::setMode(Mode mode) {
    _mode = mode;
}

void dzemikk::Skybox::render(const glm::mat4& view, const glm::mat4& projection) const {
    glDepthFunc(GL_LEQUAL);

    if (!_shader)
        return;
    _shader->bind();

    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));

    _shader->setMat4("view", viewNoTranslation);
    _shader->setMat4("projection", projection);
    _shader->setInt("mode", static_cast<int>(_mode));
    _shader->setVec3("color", _color);

    if (_mode == Mode::Cubemap) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _cubemapTex);
        _shader->setInt("skybox", 0);
    }

    _cubeMesh->draw();

    glDepthFunc(GL_LESS);
}

void dzemikk::Skybox::setShader(Shader* shader) {
    _shader = shader;
}
