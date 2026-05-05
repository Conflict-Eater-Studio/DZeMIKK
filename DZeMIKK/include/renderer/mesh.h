#ifndef DZEMIKK_MESH_H
#define DZEMIKK_MESH_H

#include <cstdint>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "assetManager/iGpuUploadable.h"

namespace dzemikk {

/**
 * @brief Represents a GPU-backed mesh with vertex and optional index data.
 *
 * Encapsulates OpenGL buffers (VAO, VBO, EBO), provides rendering methods,
 * and maintains basic geometric bounds for the mesh.
 */
class Mesh: public IGpuUploadable {
  public:

    /**
     * @brief Default constructor.
     */
    Mesh() = default;

    /**
     * @brief Virtual destructor responsible for resource cleanup.
     */
    virtual ~Mesh();

#pragma region Disable copy and move

    Mesh(const Mesh& other) = delete;
    Mesh(Mesh&& other) noexcept = delete;
    Mesh& operator=(const Mesh& other) = delete;
    Mesh& operator=(Mesh&& other) noexcept = delete;

#pragma endregion

#pragma region Rendering

    /**
     * @brief Renders the mesh using currently bound OpenGL state.
     *
     * Uses either glDrawArrays or glDrawElements depending on whether
     * index data is present.
     */
    void draw() const;

    /**
     * @brief Renders multiple instances of the mesh.
     *
     * @param models Array of model matrices for each instance.
     * @param instanceVBO OpenGL buffer containing instance data.
     */
    void drawInstanced(const std::vector<glm::mat4>& models, GLuint instanceVBO) const;

#pragma endregion

#pragma region State queries

    /**
     * @brief Checks whether the mesh has valid GPU buffers.
     *
     * @return true if VAO/VBO are initialized, false otherwise.
     */
    [[nodiscard]] bool isValid() const;

    /**
     * @brief Returns the VAO handle for the mesh.
     *
     * @return GLuint OpenGL VAO handle.
     */
    [[nodiscard]] GLuint getVAO() const {
        return _vao;
    }

    /**
     * @brief Returns the VBO handle for the mesh.
     *
     * @return GLuint OpenGL VBO handle.
     */
    [[nodiscard]] GLuint getVBO() const {
        return _vbo;
    }

    /**
     * @brief Returns the EBO handle for the mesh.
     *
     * @return GLuint OpenGL EBO handle.
     */
    [[nodiscard]] GLuint getEBO() const {
        return _ebo;
    }

    /**
     * @brief Returns the number of vertices in the mesh.
     *
     * @return uint32_t Vertex count.
     */
    [[nodiscard]] uint32_t getVertexCount() const {
        return _vertexCount;
    }

    /**
     * @brief Returns the number of indices in the mesh.
     *
     * @return uint32_t Index count.
     */
    [[nodiscard]] uint32_t getIndexCount() const {
        return _indexCount;
    }

#pragma endregion

#pragma region Data setup

    /**
     * @brief Assigns vertex buffer data to the mesh.
     *
     * @param vao Vertex Array Object handle.
     * @param vbo Vertex Buffer Object handle.
     * @param vertexCount Number of vertices stored in the buffer.
     */
    void setVertexData(GLuint vao, GLuint vbo, uint32_t vertexCount);

    /**
     * @brief Assigns index buffer data to the mesh.
     *
     * @param ebo Element Buffer Object handle.
     * @param indexCount Number of indices.
     */
    void setIndexData(GLuint ebo, uint32_t indexCount);

#pragma endregion

#pragma region Bounds

    /**
     * @brief Returns the minimum corner of the mesh bounding box.
     *
     * @return glm::vec3 Minimum bounds.
     */
    [[nodiscard]] glm::vec3 getBoundsMin() const {
        return _boundsMin;
    }

    /**
     * @brief Returns the maximum corner of the mesh bounding box.
     *
     * @return glm::vec3 Maximum bounds.
     */
    [[nodiscard]] glm::vec3 getBoundsMax() const {
        return _boundsMax;
    }

    /**
     * @brief Computes axis-aligned bounding box from vertex positions.
     *
     * @param positions Array of vertex positions.
     */
    void computeBounds(const std::vector<glm::vec3>& positions);

#pragma endregion

#pragma region Lifetime

    /**
     * @brief Releases all GPU resources associated with the mesh.
     *
     * Deletes VAO, VBO, and EBO buffers.
     */
    void destroy();

#pragma endregion

    void uploadToGPU() override {};
  protected:
#pragma region GPU resources

    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;

#pragma endregion

#pragma region Mesh data

    uint32_t _vertexCount = 0;
    uint32_t _indexCount = 0;
    bool _useIndices = false;

#pragma endregion

#pragma region Bounds data

    glm::vec3 _boundsMin{0.0F};
    glm::vec3 _boundsMax{0.0F};

#pragma endregion

#pragma region Internal methods

    /**
     * @brief Initializes and uploads index buffer data.
     *
     * @param indices Pointer to index array.
     * @param indexCount Number of indices.
     */
    void setupIndexBuffer(const unsigned int* indices, uint32_t indexCount);

#pragma endregion
};

} // namespace dzemikk

#endif // DZEMIKK_MESH_H