#include "mesh.h"

namespace dzemikk {
struct SkinnedVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    int boneIDs[4];
    float weights[4];
};

class SkinnedMesh : public Mesh {
  public:
    void create(const std::vector<SkinnedVertex>& vertices,
                const std::vector<unsigned int>& indices) {

        _vertexCount = vertices.size();
        _indexCount = indices.size();
        _useIndices = (_indexCount > 0);

        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SkinnedVertex), vertices.data(),
                     GL_STATIC_DRAW);

        if (_useIndices) {
            setupIndexBuffer(indices.data(), _indexCount);
        }

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                              (void*)offsetof(SkinnedVertex, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                              (void*)offsetof(SkinnedVertex, normal));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                              (void*)offsetof(SkinnedVertex, uv));
        glEnableVertexAttribArray(2);

        // bone IDs
        glVertexAttribIPointer(3, 4, GL_INT, sizeof(SkinnedVertex),
                               (void*)offsetof(SkinnedVertex, boneIDs));
        glEnableVertexAttribArray(3);

        // weights
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                              (void*)offsetof(SkinnedVertex, weights));
        glEnableVertexAttribArray(4);

        glBindVertexArray(0);

        std::vector<glm::vec3> positions;
        positions.reserve(vertices.size());

        for (const auto& v : vertices) {
            positions.push_back(v.position);
        }

        computeBounds(positions);
    }

    void recreate(const std::vector<SkinnedVertex>& vertices,
                               const std::vector<unsigned int>& indices) {
        destroy();

        _vertexCount = vertices.size();
        _indexCount = indices.size();
        _useIndices = (_indexCount > 0);

        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SkinnedVertex), vertices.data(),
                     GL_STATIC_DRAW);

        if (_useIndices) {
            setupIndexBuffer(indices.data(), _indexCount);
        }

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                              (void*)offsetof(SkinnedVertex, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                              (void*)offsetof(SkinnedVertex, normal));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                              (void*)offsetof(SkinnedVertex, uv));
        glEnableVertexAttribArray(2);

        // bone IDs
        glVertexAttribIPointer(3, 4, GL_INT, sizeof(SkinnedVertex),
                               (void*)offsetof(SkinnedVertex, boneIDs));
        glEnableVertexAttribArray(3);

        // weights
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                              (void*)offsetof(SkinnedVertex, weights));
        glEnableVertexAttribArray(4);

        glBindVertexArray(0);

        std::vector<glm::vec3> positions;
        positions.reserve(vertices.size());

        for (const auto& v : vertices) {
            positions.push_back(v.position);
        }

        computeBounds(positions);
    }
};
}