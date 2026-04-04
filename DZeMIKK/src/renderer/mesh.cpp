#include "renderer/mesh.h"

void dzemikk::Mesh::draw() const {
    glBindVertexArray(vao);

    if (useIndices) {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    glBindVertexArray(0);
}