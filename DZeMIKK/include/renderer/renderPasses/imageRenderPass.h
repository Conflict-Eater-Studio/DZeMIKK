#ifndef DZEMIKK_IMAGE_RENDER_PASS_H
#define DZEMIKK_IMAGE_RENDER_PASS_H

#include "renderer/renderPasses/IRenderPass.h"

namespace dzemikk {

class ImageRenderer;

/**
 * @brief Render pass responsible for rendering UI images (2D UI sprites).
 *
 * This pass handles ImageRenderer components, which are typically used
 * for UI elements such as icons, panels, HUD elements, and textures
 * drawn in screen space.
 *
 * Rendering is performed using the UI camera and orthographic projection.
 */
class ImageRenderPass : public IRenderPass {
  public:
    /**
     * @brief Executes UI image rendering stage.
     *
     * Responsibilities:
     * - collect all active ImageRenderer components
     * - render them in correct UI order (layering / sorting if needed)
     * - use UI projection matrix for screen-space rendering
     *
     * @param ctx Shared render context containing UI camera and projection.
     */
    void execute(RenderContext& ctx) override;

  private:
    /**
     * @brief List of UI image renderers scheduled for rendering.
     *
     * Typically populated via ECS system before the render pass executes.
     */
    std::vector<ImageRenderer*> _uiImages;
};

} // namespace dzemikk

#endif // DZEMIKK_IMAGE_RENDER_PASS_H