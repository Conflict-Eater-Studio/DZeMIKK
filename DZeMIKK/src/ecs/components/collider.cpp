#include "ecs/components/collider.h"
#include "renderer/model.h"
#include "renderer/mesh.h"

namespace dzemikk {

void Collider::setModel(AssetHandle<Model> model) {
    _model = model;
    calculateCullingRadius(model.get());
}

Model* Collider::getModel() const {
    return _model.get();
}
AssetHandle<Model> Collider::getModelHandle() {
    return _model;
}
void Collider::setTransform(Transform* transform) {
    _transform = transform;
}

Transform* Collider::getTransform() const {
    return _transform;
}

float Collider::getCullingRadius() const {
    return _cullingRadius;
}

void Collider::calculateCullingRadius(Model* model) {
    if (!model || model->getSubMeshes().empty()) {
        return;
    }

    glm::vec3 globalMin(FLT_MAX);
    glm::vec3 globalMax(-FLT_MAX);

    for (const auto& mesh : model->getSubMeshes()) {
        if (!mesh.mesh) continue;
        glm::vec3 min = mesh.mesh->getBoundsMin();
        glm::vec3 max = mesh.mesh->getBoundsMax();

        globalMin = glm::min(globalMin, min);
        globalMax = glm::max(globalMax, max);
    }

    glm::vec3 size = globalMax - globalMin;
    _cullingRadius = glm::length(size) * 0.5f;
}

} // namespace dzemikk
