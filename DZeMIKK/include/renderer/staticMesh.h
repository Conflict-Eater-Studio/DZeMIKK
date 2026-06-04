#ifndef DZEMIKK_STATIC_MESH_H
#define DZEMIKK_STATIC_MESH_H
#include "mesh.h"

namespace dzemikk {

/**
 * @brief Vertex format used by StaticMesh.
 *
 * Defines a standard static geometry vertex layout consisting of:
 * position, normal, and texture coordinate (UV).
 */
struct StaticVertex {
    glm::vec3 position; ///< Vertex position in model space.
    glm::vec3 normal;   ///< Vertex normal vector for lighting.
    glm::vec2 uv;       ///< Texture coordinates.
};

/**
 * @brief Concrete mesh type for static (non-deforming) geometry.
 *
 * StaticMesh uploads vertex and index data to the GPU once and is intended
 * for objects that do not change their topology at runtime.
 */
class StaticMesh : public Mesh {
  public:
#pragma region Mesh lifecycle

    /**
     * @brief Creates and uploads mesh data to the GPU.
     *
     * Initializes VAO/VBO/EBO, configures vertex attributes,
     * and computes bounding volumes.
     *
     * @param vertices Array of vertex data.
     * @param indices Index buffer defining triangle topology.
     */
    void create(const std::vector<StaticVertex>& vertices,
                const std::vector<unsigned int>& indices);

    /**
     * @brief Recreates mesh data by destroying previous GPU state and uploading new data.
     *
     * Fully reinitializes the mesh buffers and recomputes bounds.
     *
     * @param vertices Array of vertex data.
     * @param indices Index buffer defining triangle topology.
     */
    void recreate(const std::vector<StaticVertex>& vertices,
                  const std::vector<unsigned int>& indices);

    /**
     * @brief Uploads resource data to the GPU.
     *
     * Transfers CPU-side asset data into GPU memory so it can be used
     * for rendering. Called after the asset has been fully loaded.
     */
    void uploadToGPU() override;
#pragma endregion

  private:
    std::vector<StaticVertex> _vertices;
    std::vector<unsigned int> _indices;
};

} // namespace dzemikk

#endif // DZEMIKK_STATIC_MESH_H