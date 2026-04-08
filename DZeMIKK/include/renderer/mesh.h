#ifndef DZEMIKK_MESH_H
#define DZEMIKK_MESH_H

#include <cstdint>
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

namespace dzemikk {

    /**
     * @brief Represents a GPU mesh with vertex and index buffers.
     *
     * Mesh encapsulates OpenGL buffers (VAO, VBO, EBO) and provides
     * a draw interface used by the rendering system.
     */
    class Mesh {
    public:
        /**
         * @brief Constructs an empty mesh.
         */
        Mesh() = default;

        /**
         * @brief Destroys the mesh and releases GPU resources.
         */
        ~Mesh();

        #pragma region Disable copy and move

        Mesh(const Mesh& other) = delete;
        Mesh(Mesh&& other) noexcept = delete;
        Mesh& operator=(const Mesh& other) = delete;
        Mesh& operator=(Mesh&& other) noexcept = delete;

        #pragma endregion

        #pragma region Getters


        /**
         * @brief Returns the VAO handle for the mesh.
         *
         * @return GLuint OpenGL VAO handle.
         */
        [[nodiscard]] GLuint getVAO() const {
            return _vao;
        }

        /**
         * @brief Returns the number of vertices in the mesh.
         */
        [[nodiscard]] uint32_t getVertexCount() const {
            return _vertexCount;
        }

        /**
         * @brief Returns the number of indices in the mesh (if using EBO).
         */
        [[nodiscard]] uint32_t getIndexCount() const {
            return _indexCount;
        }

        /**
         * @brief Checks whether the mesh uses an index buffer.
         */
        [[nodiscard]] bool usesIndices() const {
            return _useIndices;
        }

        #pragma endregion

        #pragma region Setup

        /**
         * @brief Sets vertex buffer data.
         *
         * @param vao Vertex array object.
         * @param vbo Vertex buffer object.
         * @param vertexCount Number of vertices.
         */
        void setVertexData(GLuint vao, GLuint vbo, uint32_t vertexCount);

        /**
         * @brief Sets index buffer data and EBO handle.
         *
         * @param ebo Element Buffer Object handle.
         * @param indexCount Number of indices.
         */
        void setIndexData(GLuint ebo, uint32_t indexCount);

        #pragma endregion

        #pragma region Rendering

        /**
         * @brief Draws the mesh using the current OpenGL context.
         *
         * Uses glDrawArrays or glDrawElements depending on `_useIndices`.
         */
        void draw() const;

        
        /**
         * @brief Draws multiple instances of the mesh.
         *
         * Uses instanced rendering with a pre-allocated instance VBO.
         *
         * @param models Vector of model matrices for each instance.
         * @param instanceVBO OpenGL buffer handle used for instance data.
         */
        void drawInstanced(const std::vector<glm::mat4>& models, GLuint instanceVBO) const;
        #pragma endregion

        #pragma region Validation

        /**
         * @brief Checks if the mesh is ready to render.
         *
         * A mesh is valid if VAO is allocated and has vertices.
         *
         * @return true if valid, false otherwise.
         */
        [[nodiscard]] bool isValid() const {
            return _vao != 0 && _vertexCount > 0;
        }

        #pragma endregion

        #pragma region Creation

        /**
         * @brief Creates a mesh from raw vertex data.
         *
         * @param vertices Pointer to float array containing vertex data.
         * @param vertexCount Number of vertices.
         * @param stride Size of a single vertex in floats (e.g., 6 for position+normal).
         */
        void create(const float* vertices, uint32_t vertexCount, uint32_t stride);

        /**
         * @brief Creates a mesh with an index buffer from raw data.
         *
         * @param vertices Pointer to vertex data.
         * @param vertexCount Number of vertices.
         * @param indices Pointer to index data.
         * @param indexCount Number of indices.
         * @param stride Size of a single vertex in floats.
         */
        void createIndexed(const float* vertices, uint32_t vertexCount, const unsigned int* indices,
                           uint32_t indexCount, uint32_t stride);

        #pragma endregion

            
        glm::vec3 getBoundsMin() const {
            return _boundsMin;
        }
        glm::vec3 getBoundsMax() const {
            return _boundsMax;
        }

        void computeBounds(const float* vertices, uint32_t vertexCount, uint32_t stride) {
            if (vertexCount == 0)
                return;

            _boundsMin = glm::vec3(vertices[0], vertices[1], vertices[2]);
            _boundsMax = _boundsMin;

            for (uint32_t i = 1; i < vertexCount; ++i) {
                float x = vertices[i * stride + 0];
                float y = vertices[i * stride + 1];
                float z = vertices[i * stride + 2];

                _boundsMin = glm::min(_boundsMin, glm::vec3(x, y, z));
                _boundsMax = glm::max(_boundsMax, glm::vec3(x, y, z));
            }
        }

    private:
        #pragma region GPU Handles

        GLuint _vao = 0;
        GLuint _vbo = 0;
        GLuint _ebo = 0;

        #pragma endregion

        #pragma region Data

        uint32_t _vertexCount = 0;
        uint32_t _indexCount = 0;

        bool _useIndices = false;

        glm::vec3 _boundsMin{0.0f};
        glm::vec3 _boundsMax{0.0f};

        #pragma endregion
    };

} // namespace dzemikk

#endif // DZEMIKK_MESH_H