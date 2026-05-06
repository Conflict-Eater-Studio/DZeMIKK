#ifndef DZEMIKK_SKINNED_RENDER_PASS_H
#define DZEMIKK_SKINNED_RENDER_PASS_H

#include "renderer/renderPasses/IRenderPass.h"

namespace dzemikk {

class SkinnedMeshRenderer;

/**
 * @brief Render pass responsible for rendering skinned (animated) meshes.
 *
 * This pass handles GPU skinning or CPU-prepared skeletal animation data
 * and renders animated characters or deformable meshes.
 *
 * Typically executed after static mesh rendering, but before transparent/UI passes.
 */
class SkinnedRenderPass : public IRenderPass {
  public:
    /**
     * @brief Executes skinned mesh rendering stage.
     *
     * Responsibilities:
     * - collect skinned mesh renderers from scene
     * - update bone matrices (if required on CPU side)
     * - bind animation data to shaders
     * - issue draw calls for animated meshes
     *
     * @param ctx Shared render context containing camera and frame data.
     */
    void execute(RenderContext& ctx) override;

  private:
    /**
     * @brief List of all active skinned mesh renderers for this frame.
     *
     * Typically populated during scene traversal or ECS query phase.
     */
    std::vector<SkinnedMeshRenderer*> _skinnedRenderers;
};

} // namespace dzemikk

#endif // DZEMIKK_SKINNED_RENDER_PASS_H