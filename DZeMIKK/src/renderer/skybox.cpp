#include "renderer/skybox.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

dzemikk::Skybox::Skybox() {
    initCube();
    const char* vertexSrc = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    out vec3 TexCoords;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        TexCoords = aPos;
        vec4 pos = projection * view * vec4(aPos, 1.0);
        gl_Position = pos.xyww; // aby depth by³ zawsze 1
    }
    )";

    const char* fragmentSrc = R"(
    #version 330 core
    in vec3 TexCoords;
    out vec4 FragColor;
    uniform samplerCube skybox;
    uniform vec3 color;
    uniform bool useCubemap;
    void main() {
        if (useCubemap)
            FragColor = texture(skybox, TexCoords);
        else
            FragColor = vec4(color, 1.0);
    }
    )";

    _shader = new Shader(vertexSrc, fragmentSrc);
}

dzemikk::Skybox::~Skybox() {
    delete _cubeMesh;
    delete _shader;
    if (_cubemapTex != 0)
        glDeleteTextures(1, &_cubemapTex);
}

void dzemikk::Skybox::initCube() {
    float vertices[] = {// cube positions
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

    _cubeMesh = new Mesh();
    _cubeMesh->create(vertices, 36, 3);
}

void dzemikk::Skybox::loadCubemap(const std::vector<std::string>& faces) {
    glGenTextures(1, &_cubemapTex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _cubemapTex);

    stbi_set_flip_vertically_on_load(false);

    int width, height, nrChannels;

    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

        if (data) {
            GLenum format;

            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;
            else {
                std::cerr << "Unsupported channel count in: " << faces[i] << "\n";
                stbi_image_free(data);
                continue;
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format,
                         GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        } else {
            std::cerr << "Cubemap texture failed to load at: " << faces[i] << "\n";
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    _useCubemap = true;
}

void dzemikk::Skybox::setColor(const glm::vec3& color) {
    _color = color;
    _useCubemap = false;
}

void dzemikk::Skybox::render(const glm::mat4& view, const glm::mat4& projection) {
    glDepthFunc(GL_LEQUAL); 
    _shader->bind();
    glm::mat4 viewNoTrans = glm::mat4(glm::mat3(view)); 
    _shader->setMat4("view", viewNoTrans);
    _shader->setMat4("projection", projection);
    _shader->setBool("useCubemap", _useCubemap);
    _shader->setVec3("color", _color);

    if (_useCubemap) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _cubemapTex);
        _shader->setInt("skybox", 0);
    }

    _cubeMesh->draw();
    glDepthFunc(GL_LESS);
}
