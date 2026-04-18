#include "ecs/components/skinnedMeshRenderer.h"


void dzemikk::SkinnedMeshRenderer::calculateBoneMatrices(int index, const glm::mat4& parentTransform) {
    const Bone* bone = _model->getSkeleton()->getBone(index);

    glm::mat4 global = parentTransform * bone->getLocalTransform();

    _finalBoneMatrices[index] =
        _model->getSkeleton()->getGlobalInverseTransform() * global * bone->getOffsetMatrix();

    for (int child : bone->getChildren()) {
        calculateBoneMatrices(child, global);
    }
}
