#ifndef DZEMIKK_POST_PROCESS_RENDER_PASS_H
#define DZEMIKK_POST_PROCESS_RENDER_PASS_H

#include "renderer/renderPasses/iRenderPass.h"

#include <vector>
#include <renderer/framebuffer.h>

namespace dzemikk {

class PostProcessEffect;

class PostProcessRenderPass : public IRenderPass {
  public:
    PostProcessRenderPass();
    ~PostProcessRenderPass() override;

    void execute(RenderContext& ctx) override;

  private:
    void initializeQuad();
    void destroyQuad();

  private:
    GLuint _vao = 0;
    GLuint _vbo = 0;
    
    std::unique_ptr<Framebuffer> _pingFramebuffer;
    std::unique_ptr<Framebuffer> _pongFramebuffer;

    std::vector<PostProcessEffect*> _effects;
};

} // namespace dzemikk

#endif