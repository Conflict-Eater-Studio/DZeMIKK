#ifndef DZEMIKK_MESH_RENDER_PASS_H
#define DZEMIKK_MESH_RENDER_PASS_H

#include "ecs/components/meshRenderer.h"
#include "renderer/material.h"
#include "renderer/renderPasses/IRenderPass.h"

#include <glad.h>

namespace dzemikk {

class Mesh;

/**
 * @brief Render pass responsible for rendering static (non-skinned) meshes.
 *
 * This pass collects all active MeshRenderer components from the scene,
 * groups them into batches based on shared Mesh + Material, and renders them
 * using instanced rendering where possible.
 *
 * Goals of this pass:
 * - reduce draw calls via batching
 * - support instanced rendering for repeated meshes
 * - apply per-object transform data efficiently via instance buffers
 */
class MeshRenderPass : public IRenderPass {
  public:
    /**
     * @brief Executes full mesh rendering stage.
     *
     * Pipeline:
     * 1. Collect visible MeshRenderer components
     * 2. Build rendering batches (mesh + material grouping)
     * 3. Upload instance data (model matrices)
     * 4. Render all batches
     *
     * @param ctx Shared render context (cameras, matrices, frustum)
     */
    void execute(RenderContext& ctx) override;

    /**
     * @brief Destructor responsible for GPU cleanup.
     *
     * Frees instance buffers (VBOs) allocated for batching.
     */
    ~MeshRenderPass() override;

  private:
    /**
     * @brief Represents a single instanced rendering batch.
     *
     * All objects in a batch share:
     * - same mesh geometry
     * - same material/shader
     *
     * Each instance differs by transformation matrix.
     */
    struct Batch {

        /**
         * @brief Mesh geometry used for this batch.
         */
        Mesh* mesh = nullptr;

        /**
         * @brief Material applied to all instances in this batch.
         */
        Material* material = nullptr;

        /**
         * @brief Model matrices for each instance in the batch.
         */
        std::vector<glm::mat4> models;

        /**
         * @brief OpenGL buffer storing instance transformation data.
         */
        GLuint instanceVBO = 0;

        /**
         * @brief Optional per-batch color tint.
         */
        glm::vec3 color = glm::vec3(1.0f);
    };

    /**
     * @brief Finalized batches ready for GPU rendering.
     */
    std::vector<Batch> _batches;

    /**
     * @brief Cached list of scene mesh renderers collected each frame.
     *
     * Typically filled during buildMeshBatches().
     */
    std::vector<MeshRenderer*> _meshRenderers;

    /**
     * @brief Collects visible mesh renderers and groups them into batches.
     *
     * Uses frustum culling from RenderContext if available.
     */
    void buildMeshBatches(RenderContext& ctx);

    /**
     * @brief Renders all prepared mesh batches.
     *
     * Issues instanced draw calls and binds required GPU resources.
     */
    void renderMeshBatches(RenderContext& ctx);
};

} // namespace dzemikk
#endif // DZEMIKK_MESH_RENDER_PASS_H