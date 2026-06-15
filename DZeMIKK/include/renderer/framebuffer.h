#ifndef DZEMIKK_FRAMEBUFFER_H
#define DZEMIKK_FRAMEBUFFER_H

#include <cstdint>

namespace dzemikk {

class Framebuffer {
  public:
    Framebuffer(uint32_t width, uint32_t height);
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    Framebuffer(Framebuffer&&) = delete;
    Framebuffer& operator=(Framebuffer&&) = delete;

    void bind() const;
    void unbind() const;

    void resize(uint32_t width, uint32_t height);

    [[nodiscard]] uint32_t getRendererID() const;
    [[nodiscard]] uint32_t getColorAttachmentRendererID() const;
    [[nodiscard]] uint32_t getDepthAttachmentRendererID() const;

    [[nodiscard]] uint32_t getWidth() const;
    [[nodiscard]] uint32_t getHeight() const;

  private:
    void invalidate();

  private:
    uint32_t _rendererID = 0;

    uint32_t _colorAttachment = 0;
    uint32_t _depthAttachment = 0;

    uint32_t _width = 0;
    uint32_t _height = 0;
};

} // namespace dzemikk

#endif