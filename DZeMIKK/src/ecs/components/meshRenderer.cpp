#include "ecs/components/meshRenderer.h"
#include "renderer/model.h"

namespace dzemikk {
void MeshRenderer::setCullingRadius(float cullingRadius) {
    _cullingRadius = cullingRadius;
}

void MeshRenderer::calculateCullingRadius(Model* model) {
    if (!model || model->getSubMeshes().empty()) {
        return;
    }

    glm::vec3 globalMin(FLT_MAX);
    glm::vec3 globalMax(-FLT_MAX);

    for (const auto& mesh : model->getSubMeshes()) {
        glm::vec3 min = mesh.mesh->getBoundsMin();
        glm::vec3 max = mesh.mesh->getBoundsMax();

        globalMin = glm::min(globalMin, min);
        globalMax = glm::max(globalMax, max);
    }

    glm::vec3 size = globalMax - globalMin;
    _cullingRadius = glm::length(size) * 0.5F;
}


float MeshRenderer::getCullingRadius() const {
    return _cullingRadius;
}
}