#include "ecs/components/skinnedMeshRenderer.h"

void dzemikk::SkinnedMeshRenderer::calculateBoneMatrices(int index,
                                                         const glm::mat4& parentTransform) {
    auto skeleton = getSkeleton();
    if (!skeleton) {
        return;
    }

    const Bone* bone = skeleton->getBone(index);

    glm::mat4 local = bone->getLocalTransform();

    glm::mat4 global = parentTransform * local;

    _finalBoneMatrices[index] = global * bone->getOffsetMatrix();

    for (int child : bone->getChildren()) {
        calculateBoneMatrices(child, global);
    }
}
