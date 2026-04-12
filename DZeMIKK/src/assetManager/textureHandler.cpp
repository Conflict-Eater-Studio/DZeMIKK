#include "assetManager/textureHandler.h"

#include "renderer/texture.h"

#include <stb/stb_image.h>
#include <iostream>

void* dzemikk::TextureHandler::load(const std::string& path) {
    return loadTextureFromFile(path);
}

dzemikk::Texture* dzemikk::TextureHandler::loadTextureFromFile(const std::string& path, bool flipVertical) {
    int width, height, channels;

    stbi_set_flip_vertically_on_load(flipVertical ? 1 : 0);

    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return nullptr;
    }

    auto texture = new Texture();
    texture->initFromData(data, width, height, channels);

    stbi_image_free(data);

    return texture;
}

void dzemikk::TextureHandler::reload(void* asset, const std::string& path) {
    reloadTexture(path, static_cast<Texture*>(asset));
}

void dzemikk::TextureHandler::reloadTexture(const std::string& path, dzemikk::Texture* texture) {
    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to reload texture: " << path << "\n";
        return;
    }

    GLuint newTex;
    glGenTextures(1, &newTex);
    glBindTexture(GL_TEXTURE_2D, newTex);

    GLenum format = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_RED;
    GLenum internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE,
                 data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    texture->replaceTexture(newTex, width, height, channels);
}

void dzemikk::TextureHandler::unload(void* asset) {
    delete static_cast<Texture*>(asset);
}
