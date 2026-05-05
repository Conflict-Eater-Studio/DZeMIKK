#include "renderer/skinnedMesh.h"

constexpr auto ATTR_OFFSET = [](auto member) { return reinterpret_cast<const void*>(member); };

void dzemikk::SkinnedMesh::create(const std::vector<SkinnedVertex>& vertices,
                                  const std::vector<unsigned int>& indices) {

    _vertexCount = vertices.size();
    _indexCount = indices.size();
    _useIndices = (_indexCount > 0);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(SkinnedVertex)), vertices.data(),
                 GL_STATIC_DRAW);

    if (_useIndices) {
        setupIndexBuffer(indices.data(), _indexCount);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                           ATTR_OFFSET(offsetof(SkinnedVertex, position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                           ATTR_OFFSET(offsetof(SkinnedVertex, normal)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                           ATTR_OFFSET(offsetof(SkinnedVertex, uv)));
    glEnableVertexAttribArray(2);

    // bone IDs
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(SkinnedVertex),
                            ATTR_OFFSET(offsetof(SkinnedVertex, boneIDs)));
    glEnableVertexAttribArray(3);

    // weights
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                           ATTR_OFFSET(offsetof(SkinnedVertex, weights)));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    std::vector<glm::vec3> positions;
    positions.reserve(vertices.size());

    for (const auto& v : vertices) {
        positions.push_back(v.position);
    }

    computeBounds(positions);
}

void dzemikk::SkinnedMesh::recreate(const std::vector<SkinnedVertex>& vertices,
                                    const std::vector<unsigned int>& indices) {
    destroy();

    _vertexCount = vertices.size();
    _indexCount = indices.size();
    _useIndices = (_indexCount > 0);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(SkinnedVertex)),
                 vertices.data(),
                 GL_STATIC_DRAW);

    if (_useIndices) {
        setupIndexBuffer(indices.data(), _indexCount);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                          ATTR_OFFSET(offsetof(SkinnedVertex, position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                          ATTR_OFFSET(offsetof(SkinnedVertex, normal)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                          ATTR_OFFSET(offsetof(SkinnedVertex, uv)));
    glEnableVertexAttribArray(2);

    // bone IDs
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(SkinnedVertex),
                           ATTR_OFFSET(offsetof(SkinnedVertex, boneIDs)));
    glEnableVertexAttribArray(3);

    // weights
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex),
                          ATTR_OFFSET(offsetof(SkinnedVertex, weights)));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    std::vector<glm::vec3> positions;
    positions.reserve(vertices.size());

    for (const auto& v : vertices) {
        positions.push_back(v.position);
    }

    computeBounds(positions);
}
