#ifndef DZEMIKK_SKINNED_MESH_H
#define DZEMIKK_SKINNED_MESH_H
#include "mesh.h"
#include <array>

namespace dzemikk {

/**
 * @brief Vertex format for skinned (skeletal animated) meshes.
 *
 * Extends a standard static vertex with bone influence data used for
 * skeletal animation (skinning). Each vertex can be influenced by up to
 * 4 bones with corresponding weights.
 */
struct SkinnedVertex {
    glm::vec3 position;
    glm::vec3 normal;   
    glm::vec2 uv;   
    glm::vec3 tangent;
    glm::vec3 bitangent;

    std::array<int, 4> boneIDs;
    std::array<float, 4> weights;
};

/**
 * @brief Mesh type supporting skeletal animation (skinning).
 *
 * SkinnedMesh extends the base Mesh class with support for bone indices
 * and weights per vertex, enabling GPU skinning in the vertex shader.
 */
class SkinnedMesh : public Mesh {
  public:
#pragma region Mesh lifecycle

    /**
     * @brief Creates and uploads skinned mesh data to the GPU.
     *
     * Initializes buffers (VAO/VBO/EBO), configures vertex attributes
     * including bone IDs and weights, and uploads vertex data.
     *
     * @param vertices Array of skinned vertices.
     * @param indices Index buffer defining mesh topology.
     */
    void create(const std::vector<SkinnedVertex>& vertices,
                const std::vector<unsigned int>& indices);

    /**
     * @brief Recreates GPU resources for the skinned mesh.
     *
     * Destroys previous GPU state and reinitializes buffers and attributes
     * with new vertex and index data.
     *
     * @param vertices Array of skinned vertices.
     * @param indices Index buffer defining mesh topology.
     */
    void recreate(const std::vector<SkinnedVertex>& vertices,
                  const std::vector<unsigned int>& indices);

#pragma endregion
};

} // namespace dzemikk
#endif // DZEMIKK_SKINNED_MESH_H