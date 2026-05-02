#include "renderer/staticMesh.h"

constexpr auto ATTR_OFFSET = [](auto member) { return reinterpret_cast<const void*>(member); };

void dzemikk::StaticMesh::create(const std::vector<StaticVertex>& vertices, const std::vector<unsigned int>& indices) {

    _vertexCount = vertices.size();
    _indexCount = indices.size();
    _useIndices = (_indexCount > 0);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(StaticVertex)),
                 vertices.data(), GL_STATIC_DRAW);

    if (_useIndices) {
        setupIndexBuffer(indices.data(), _indexCount);
    }

    // layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(StaticVertex),
                          ATTR_OFFSET(offsetof(StaticVertex, position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(StaticVertex),
                           ATTR_OFFSET(offsetof(StaticVertex, normal)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(StaticVertex),
                           ATTR_OFFSET(offsetof(StaticVertex, uv)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    std::vector<glm::vec3> positions;
    positions.reserve(vertices.size());

    for (const auto& v : vertices) {
        positions.push_back(v.position);
    }

    computeBounds(positions);
}

void dzemikk::StaticMesh::recreate(const std::vector<StaticVertex>& vertices,
                                   const std::vector<unsigned int>& indices) {
    destroy();

    _vertexCount = vertices.size();
    _indexCount = indices.size();
    _useIndices = (_indexCount > 0);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(StaticVertex)),
                 vertices.data(), GL_STATIC_DRAW);

    if (_useIndices) {
        setupIndexBuffer(indices.data(), _indexCount);
    }

    // layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(StaticVertex),
                          ATTR_OFFSET(offsetof(StaticVertex, position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(StaticVertex),
                          ATTR_OFFSET(offsetof(StaticVertex, normal)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(StaticVertex),
                          ATTR_OFFSET(offsetof(StaticVertex, uv)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    std::vector<glm::vec3> positions;
    positions.reserve(vertices.size());

    for (const auto& v : vertices) {
        positions.push_back(v.position);
    }

    computeBounds(positions);
}