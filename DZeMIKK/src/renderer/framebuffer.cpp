#include "renderer/framebuffer.h"

#include <glad/glad.h>
#include <iostream>

namespace dzemikk {

Framebuffer::Framebuffer(uint32_t width, uint32_t height) : _width(width), _height(height) {

    invalidate();
}

Framebuffer::~Framebuffer() {

    if (_colorAttachment) {
        glDeleteTextures(1, &_colorAttachment);
    }

    if (_depthAttachment) {
        glDeleteRenderbuffers(1, &_depthAttachment);
    }

    if (_rendererID) {
        glDeleteFramebuffers(1, &_rendererID);
    }
}

void Framebuffer::invalidate() {

    if (_rendererID) {

        glDeleteFramebuffers(1, &_rendererID);
        glDeleteTextures(1, &_colorAttachment);
        glDeleteRenderbuffers(1, &_depthAttachment);

        _rendererID = 0;
        _colorAttachment = 0;
        _depthAttachment = 0;
    }

    glGenFramebuffers(1, &_rendererID);
    glBindFramebuffer(GL_FRAMEBUFFER, _rendererID);

    glGenTextures(1, &_colorAttachment);
    glBindTexture(GL_TEXTURE_2D, _colorAttachment);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<int>(_width), static_cast<int>(_height), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorAttachment,
                           0);

    glGenRenderbuffers(1, &_depthAttachment);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthAttachment);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<int>(_width),
                          static_cast<int>(_height));

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              _depthAttachment);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {

        std::cerr << "Framebuffer incomplete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind() const {

    glBindFramebuffer(GL_FRAMEBUFFER, _rendererID);
}

void Framebuffer::unbind() const {

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(uint32_t width, uint32_t height) {

    if (width == 0 || height == 0) {
        return;
    }

    _width = width;
    _height = height;

    invalidate();
}

uint32_t Framebuffer::getRendererID() const {

    return _rendererID;
}

uint32_t Framebuffer::getColorAttachmentRendererID() const {

    return _colorAttachment;
}

uint32_t Framebuffer::getWidth() const {

    return _width;
}

uint32_t Framebuffer::getHeight() const {

    return _height;
}

} // namespace dzemikk