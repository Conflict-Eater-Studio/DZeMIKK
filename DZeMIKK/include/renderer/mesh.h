#ifndef DZEMIKK_MESH_H
#define DZEMIKK_MESH_H

#include <cstdint>
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

namespace dzemikk {

class Mesh {
  public:
    virtual ~Mesh();

    void draw() const;
    void drawInstanced(const std::vector<glm::mat4>& models, GLuint instanceVBO) const;

    [[nodiscard]] bool isValid() const;

            /**
     * @brief Returns the VAO handle for the mesh.
     *
     * @return GLuint OpenGL VAO handle.
     */
    [[nodiscard]] GLuint getVAO() const {
        return _vao;
    }

    [[nodiscard]] GLuint getVBO() const {
        return _vbo;
    }

    [[nodiscard]] GLuint getEBO() const {
        return _ebo;
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


    void setVertexData(GLuint vao, GLuint vbo, uint32_t vertexCount);
    void setIndexData(GLuint ebo, uint32_t indexCount); 

    glm::vec3 getBoundsMin() const {
        return _boundsMin;
    }

    glm::vec3 getBoundsMax() const {
        return _boundsMax;
    }

    inline void computeBounds(const std::vector<glm::vec3>& positions) {
        if (positions.empty())
            return;

        _boundsMin = positions[0];
        _boundsMax = positions[0];

        for (const auto& p : positions) {
            _boundsMin = glm::min(_boundsMin, p);
            _boundsMax = glm::max(_boundsMax, p);
        }
    }

    void destroy();
  protected:
    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;

    uint32_t _vertexCount = 0;
    uint32_t _indexCount = 0;
    bool _useIndices = false;

    glm::vec3 _boundsMin{0.0f};
    glm::vec3 _boundsMax{0.0f};


  protected:
    void setupIndexBuffer(const unsigned int* indices, uint32_t indexCount);
};

} // namespace dzemikk

#endif // DZEMIKK_MESH_H