#pragma once

#include "assetManager/assetHandle.h"
#include "renderer/framebuffer.h"
#include "renderer/renderPasses/iRenderPass.h"
#include "renderer/shader.h"

namespace dzemikk {
class Engine;
class BloomRenderPass : public IRenderPass {
  public:
    BloomRenderPass();
    ~BloomRenderPass() override;

    void initialize();
    void execute(RenderContext& ctx) override;
    void setEngine(Engine* engine) {
        _engine = engine;
    }

  private:
    void initializeQuad();
    void destroyQuad();

  private:
    GLuint _vao = 0;
    GLuint _vbo = 0;

    std::unique_ptr<Framebuffer> _brightFramebuffer;
    std::unique_ptr<Framebuffer> _pingFramebuffer;
    std::unique_ptr<Framebuffer> _pongFramebuffer;

    AssetHandle<Shader> _brightShader;
    AssetHandle<Shader> _blurShader;
    AssetHandle<Shader> _compositeShader;

    Engine* _engine = nullptr;
};

} // namespace dzemikk