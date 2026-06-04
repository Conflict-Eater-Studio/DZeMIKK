#ifndef DZEMIKK_SPRITE_RENDER_PASS_H
#define DZEMIKK_SPRITE_RENDER_PASS_H

#include "renderer/renderPasses/IRenderPass.h"

namespace dzemikk {

class SpriteRenderer;

/**
 * @brief Render pass responsible for rendering 2D sprites in world or screen space.
 *
 * This pass handles all SpriteRenderer components and draws them using
 * an orthographic projection (UI camera or world-aligned billboard system).
 *
 * It is typically executed after 3D geometry passes and before UI text.
 */
class SpriteRenderPass : public IRenderPass {
  public:
    /**
     * @brief Executes sprite rendering stage.
     *
     * Responsibilities:
     * - gather all active SpriteRenderer components
     * - render sprites using world projection
     *
     * @param ctx Shared render context containing UI camera and projection.
     */
    void execute(RenderContext& ctx) override;

  private:
    /**
     * @brief List of sprite renderers to be drawn this frame.
     *
     * Usually filled via ECS query before rendering.
     */
    std::vector<SpriteRenderer*> _spriteRenderers;
};

} // namespace dzemikk

#endif // DZEMIKK_SPRITE_RENDER_PASS_H