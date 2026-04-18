#include "renderer/mesh.h"
#include <GLFW/glfw3.h>

dzemikk::Mesh::~Mesh() {
    if (!glfwGetCurrentContext())
        return;

    destroy();
}

void dzemikk::Mesh::setVertexData(GLuint vao, GLuint vbo, uint32_t vertexCount) {
    _vao = vao;
    _vbo = vbo;
    _vertexCount = vertexCount;
}

void dzemikk::Mesh::setIndexData(GLuint ebo, uint32_t indexCount) {
    _ebo = ebo;
    _indexCount = indexCount;
    _useIndices = true;
}

void dzemikk::Mesh::setupIndexBuffer(const unsigned int* indices, uint32_t indexCount) {
    _indexCount = indexCount;
    _useIndices = (indexCount > 0);

    if (!_useIndices)
        return;

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices,
                 GL_STATIC_DRAW);
}

void dzemikk::Mesh::draw() const {
    if (!isValid())
        return; 

    glBindVertexArray(_vao);

    if (_useIndices) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_indexCount), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(_vertexCount));
    }
}

void dzemikk::Mesh::drawInstanced(const std::vector<glm::mat4>& models, GLuint instanceVBO) const {
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), models.data(),
                 GL_DYNAMIC_DRAW);

    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                              (void*)(sizeof(glm::vec4) * i));
        glEnableVertexAttribArray(2 + i);
        glVertexAttribDivisor(2 + i, 1);
    }

    if (_useIndices) {
        glDrawElementsInstanced(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, 0, models.size());
    } else {
        glDrawArraysInstanced(GL_TRIANGLES, 0, _vertexCount, models.size());
    }
    glBindVertexArray(0);
}

bool dzemikk::Mesh::isValid() const {
    return _vao != 0 && _vertexCount > 0;
}

void dzemikk::Mesh::destroy() {
    if (_ebo) {
        glDeleteBuffers(1, &_ebo);
        _ebo = 0;
    }
    if (_vbo) {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }
    if (_vao) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }

    _vertexCount = 0;
    _indexCount = 0;
    _useIndices = false;
}