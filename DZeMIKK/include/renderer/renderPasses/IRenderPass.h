#ifndef DZEMIKK_I_RENDER_PASS_H
#define DZEMIKK_I_RENDER_PASS_H

#include "ecs/components/camera.h"
#include "renderer/frustum.h"
#include "renderer/lightUtils.h"

#include <glad/glad.h>

namespace dzemikk {

/**
 * @brief Shared rendering data passed to all render passes.
 *
 * RenderContext contains per-frame state required during rendering,
 * including active cameras, matrices, GPU resources, and frustum data.
 *
 * The context is updated by Renderer before executing render passes.
 * Individual passes may read or modify selected values if needed.
 */
struct RenderContext {

    /**
     * @brief Active scene camera used for 3D world rendering.
     */
    const Camera* sceneCamera = nullptr;

    /**
     * @brief Active UI camera used for overlay/UI rendering.
     */
    const Camera* uiCamera = nullptr;

    /**
     * @brief Current view matrix generated from scene camera.
     */
    glm::mat4 view = glm::mat4(1.0f);

    /**
     * @brief Current projection matrix for 3D rendering.
     */
    glm::mat4 projection = glm::mat4(1.0f);

    /**
     * @brief Orthographic projection matrix used for UI rendering.
     */
    glm::mat4 uiProjection = glm::mat4(1.0f);

    /**
     * @brief Uniform Buffer Object storing view/projection matrices.
     *
     * Shared across shaders using uniform block bindings.
     */
    unsigned int uboMatrices = 0;

    /**
     * @brief Frustum used for visibility culling.
     */
    Frustum* frustum = nullptr;

    const GPUDirectionalLight* directionalLights = nullptr;
    const GPUPointLight* pointLights = nullptr;
    const GPUSpotLight* spotLights = nullptr;

    int directionalCount = 0;
    int pointCount = 0;
    int spotCount = 0;

    GLuint sceneTexture = 0;
    GLuint depthTexture = 0;
};

/**
 * @brief Base interface for all rendering passes.
 *
 * A render pass represents a single stage of the rendering pipeline,
 * such as:
 * - mesh rendering
 * - skybox rendering
 * - sprite rendering
 * - UI rendering
 * - post-processing
 *
 * Renderer executes all registered render passes sequentially every frame.
 */
class IRenderPass {
  public:
    /**
     * @brief Executes rendering logic for this pass.
     *
     * @param ctx Shared rendering context containing current frame data.
     */
    virtual void execute(RenderContext& ctx) = 0;

    /**
     * @brief Virtual destructor for proper polymorphic cleanup.
     */
    virtual ~IRenderPass() = default;
};

} // namespace dzemikk

#endif // DZEMIKK_I_RENDER_PASS_H