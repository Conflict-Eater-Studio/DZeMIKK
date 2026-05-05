#include "assetManager/skeletonBuilder.h"

void dzemikk::SkeletonBuilder::buildSkeleton(aiNode* node, Skeleton& skeleton, int parent) {
    std::string name = node->mName.C_Str();

    glm::mat4 local = aiToGlm(node->mTransformation);

    int index = skeleton.addBone(name, parent);

    auto* bone = skeleton.getBone(index);
    if (bone) {
        bone->setLocalTransform(local);
        bone->setBindLocalTransform(local);
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        aiNode* child = node->mChildren[i];
        buildSkeleton(child, skeleton, index);
    }
}

dzemikk::Skeleton* dzemikk::SkeletonBuilder::build(const aiScene* scene) {
    auto *skeleton = new Skeleton();
    buildSkeleton(scene->mRootNode, *skeleton, -1);
    applyBoneOffsets(scene, *skeleton);

    skeleton->setGlobalInverseTransform(glm::inverse(aiToGlm(scene->mRootNode->mTransformation)));

    return skeleton;
}

glm::mat4 dzemikk::SkeletonBuilder::aiToGlm(const aiMatrix4x4& m) {
    // Assimp uses row-major matrices, GLM uses column-major.
    // This is a direct transposition mapping.
    return {m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2,
            m.a3, m.b3, m.c3, m.d3, m.a4, m.b4, m.c4, m.d4};
}

void dzemikk::SkeletonBuilder::applyBoneOffsets(const aiScene* scene, Skeleton& skeleton) {
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        const aiMesh* mesh = scene->mMeshes[m];

        for (unsigned int b = 0; b < mesh->mNumBones; b++) {
            const aiBone* bone = mesh->mBones[b];

            int boneID = skeleton.getBoneIndex(bone->mName.C_Str());
            if (boneID == -1) {
                continue;
            }

            Bone* skelBone = skeleton.getBone(boneID);
            if (!skelBone) {
                continue;
            }

            skelBone->setOffsetMatrix(aiToGlm(bone->mOffsetMatrix));
        }
    }
}