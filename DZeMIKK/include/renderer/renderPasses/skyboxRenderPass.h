#ifndef DZEMIKK_SKYBOX_RENDER_PASS_H
#define DZEMIKK_SKYBOX_RENDER_PASS_H

#include "assetManager/assetHandle.h"
#include "renderer/renderPasses/IRenderPass.h"
#include "renderer/skybox.h"

namespace dzemikk {

/**
 * @brief Render pass responsible for drawing the scene skybox.
 *
 * This pass renders a cubemap-based skybox as the background of the scene.
 * It is typically executed early in the 3D rendering pipeline so that
 * all geometry is rendered on top of it.
 *
 * The skybox is optional — if no texture is assigned, the pass becomes a no-op.
 */
class SkyboxRenderPass : public IRenderPass {
  public:
    /**
     * @brief Executes skybox rendering.
     *
     * Uses the active camera from RenderContext and renders a cubemap
     * around the viewer. 
     *
     * @param ctx Shared rendering context containing camera and GPU state.
     */
    void execute(RenderContext& ctx) override;

    /**
     * @brief Assigns a cubemap texture used for skybox rendering.
     *
     * @param skybox Asset handle to cubemap-based skybox resource.
     */
    void setSkybox(AssetHandle<Skybox> skybox);

    /**
     * @brief Returns currently assigned skybox asset.
     *
     * @return AssetHandle to skybox (may be empty if none set).
     */
    [[nodiscard]] const AssetHandle<Skybox> getSkybox() const;

  private:
    /**
     * @brief Currently active skybox asset.
     *
     * If null/empty, SkyboxRenderPass should skip rendering.
     */
    AssetHandle<Skybox> _skybox;
};

} // namespace dzemikk
#endif // DZEMIKK_SKYBOX_RENDER_PASS_H