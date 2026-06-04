#include "assetManager/textureHandler.h"

#include "assetManager/assetError.h"
#include "renderer/texture.h"

#include <iostream>
#include <stb/stb_image.h>

dzemikk::TextureHandler::Result
dzemikk::TextureHandler::load(const std::string& path,
                              LoadExecutionMode loadExecutionMode) {
    auto texture = loadTextureFromFile(path, loadExecutionMode);

    if (!texture) {
        std::cerr << "Failed to load texture: " << path << "\n";
        return {nullptr, AssetError::LoadFailed};
    }

    return {texture, AssetError::None};
}

std::shared_ptr<dzemikk::Texture> dzemikk::TextureHandler::loadTextureFromFile(
    const std::string& path, LoadExecutionMode loadExecutionMode,
                                                             bool flipVertical) {
    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(flipVertical ? 1 : 0);

    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << path << "\n";
        return nullptr;
    }

    auto texture = std::make_shared<Texture>(data, width, height, channels);

    if (loadExecutionMode == LoadExecutionMode::Sync) {
        texture->uploadToGPU();
    }

    stbi_image_free(data);

    return texture;
}

bool dzemikk::TextureHandler::reloadTexture(const std::string& path, dzemikk::Texture& texture) {
    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to reload texture: " << path << "\n";
        return false;
    }

    GLuint newTex = 0;
    glGenTextures(1, &newTex);
    glBindTexture(GL_TEXTURE_2D, newTex);

    GLenum format = GL_RED;

    if (channels == 4)
        format = GL_RGBA;
    else if (channels == 3)
        format = GL_RGB;

    GLenum internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;

    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    texture.replaceTexture(newTex, width, height, channels);

    return true;
}

bool dzemikk::TextureHandler::reload(Handle& asset, const std::string& path) {
    if (!asset)
        return false;

    return reloadTexture(path, *asset.get());
}

void dzemikk::TextureHandler::unload(Handle& asset) {
    asset = Handle{};
}