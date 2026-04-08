#include "ecs/components/meshRenderer.h"
#include "renderer/mesh.h"

void dzemikk::MeshRenderer::calculateCullingRadius(Mesh* mesh) {
    if (!mesh)
        return;

    glm::vec3 min = mesh->getBoundsMin();
    glm::vec3 max = mesh->getBoundsMax();

    glm::vec3 size = max - min;
    _cullingRadius = glm::length(size) * 0.5f;
}

float dzemikk::MeshRenderer::getCullingRadius() {
    return _cullingRadius;
}
