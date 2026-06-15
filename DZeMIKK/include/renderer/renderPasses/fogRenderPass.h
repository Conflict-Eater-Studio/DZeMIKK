#ifndef DZEMIKK_FOG_RENDER_PASS_H
#define DZEMIKK_FOG_RENDER_PASS_H

#include "renderer/renderPasses/IRenderPass.h"
#include <vector>

namespace dzemikk {

class MeshRenderer;

class FogRenderPass : public IRenderPass {
  public:
    FogRenderPass() = default;
    ~FogRenderPass() override = default;

    void execute(RenderContext& ctx) override;

  private:
    std::vector<MeshRenderer*> _meshRenderers;
};

} // namespace dzemikk

#endif // DZEMIKK_FOG_RENDER_PASS_H
