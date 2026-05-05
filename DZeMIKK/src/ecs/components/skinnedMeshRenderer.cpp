#include "ecs/components/skinnedMeshRenderer.h"

void dzemikk::SkinnedMeshRenderer::calculateBoneMatrices(int index,
                                                         const glm::mat4& parentTransform) {
    const Bone* bone = _model.get()->getSkeleton()->getBone(index);

    glm::mat4 local = bone->getLocalTransform();

    glm::mat4 global = parentTransform * local;

    _finalBoneMatrices[index] = global * bone->getOffsetMatrix();

    for (int child : bone->getChildren()) {
        calculateBoneMatrices(child, global);
    }
}
