#include "renderer/texture.h"

dzemikk::Texture::Texture() = default;

dzemikk::Texture::~Texture() {
    release();
}

void dzemikk::Texture::bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, _id);
}

void dzemikk::Texture::unbind(uint32_t slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void dzemikk::Texture::setFilter(Filter minFilter, Filter magFilter) {
    glBindTexture(GL_TEXTURE_2D, _id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, convertFilter(minFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, convertFilter(magFilter));

    glBindTexture(GL_TEXTURE_2D, 0);
}

void dzemikk::Texture::setWrap(Wrap sWrap, Wrap tWrap) {
    glBindTexture(GL_TEXTURE_2D, _id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, convertWrap(sWrap));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, convertWrap(tWrap));

    glBindTexture(GL_TEXTURE_2D, 0);
}

GLenum dzemikk::Texture::convertFilter(Filter f) const {
    switch (f) {
    case Filter::Nearest:
        return GL_NEAREST;
    case Filter::Linear:
        return GL_LINEAR;
    case Filter::LinearMipmap:
        return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_LINEAR;
}

GLenum dzemikk::Texture::convertWrap(Wrap w) const {
    switch (w) {
    case Wrap::Repeat:
        return GL_REPEAT;
    case Wrap::ClampToEdge:
        return GL_CLAMP_TO_EDGE;
    case Wrap::MirroredRepeat:
        return GL_MIRRORED_REPEAT;
    }
    return GL_REPEAT;
}

void dzemikk::Texture::release() {
    if (_id != 0) {
        glDeleteTextures(1, &_id);
        _id = 0;
    }
}

void dzemikk::Texture::initFromData(unsigned char* data, int width, int height, int channels,
                           bool generateMipmaps) {
    release();

    _width = width;
    _height = height;
    _channels = channels;

    GLenum format = GL_RGB;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    GLenum internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;

    glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE,
                 data);

    if (generateMipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);

    setFilter(Filter::LinearMipmap, Filter::Linear);
    setWrap(Wrap::Repeat, Wrap::Repeat);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void dzemikk::Texture::replaceTexture(GLuint newId, int width, int height, int channels) {
    if (_id != 0)
        glDeleteTextures(1, &_id);

    _id = newId;
    _width = width;
    _height = height;
    _channels = channels;
}