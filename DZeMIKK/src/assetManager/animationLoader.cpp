#include "assetManager/animationLoader.h"
#include "animation/animationclip.h"
#include "animation/boneTrack.h"

void dzemikk::AnimationLoader::load(const aiScene* scene, Skeleton& skeleton) {
    std::unordered_map<std::string, int> normalizedToBone;

    for (int i = 0; i < skeleton.getBoneCount(); ++i) {
        Bone* b = skeleton.getBone(i);
        if (!b)
            continue;

        std::string norm = normalizeBoneName(b->getName());
        normalizedToBone[norm] = i;
    }

    for (unsigned int a = 0; a < scene->mNumAnimations; ++a) {

        const aiAnimation* anim = scene->mAnimations[a];

        float duration = static_cast<float>(anim->mDuration);
        float tps = anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.0f;

        auto* clip = new AnimationClip(duration, tps);

        struct ChannelBundle {
            const aiNodeAnim* translation = nullptr;
            const aiNodeAnim* rotation = nullptr;
            const aiNodeAnim* scaling = nullptr;
        };

        std::unordered_map<std::string, ChannelBundle> channelMap;

        for (unsigned int j = 0; j < anim->mNumChannels; ++j) {
            const aiNodeAnim* ch = anim->mChannels[j];

            std::string rawName = ch->mNodeName.C_Str();
            std::string norm = normalizeBoneName(rawName);

            if (rawName.find("Translation") != std::string::npos) {
                channelMap[norm].translation = ch;
            } else if (rawName.find("Rotation") != std::string::npos) {
                channelMap[norm].rotation = ch;
            } else if (rawName.find("Scaling") != std::string::npos) {
                channelMap[norm].scaling = ch;
            } else {
                channelMap[norm].translation = ch;
                channelMap[norm].rotation = ch;
                channelMap[norm].scaling = ch;
            }
        }

        for (const auto& [normName, bundle] : channelMap) {

            auto it = normalizedToBone.find(normName);
            if (it == normalizedToBone.end())
                continue;

            int boneIndex = it->second;
            Bone* bone = skeleton.getBone(boneIndex);
            if (!bone)
                continue;

            BoneTrack* track = clip->addBoneTrack();
            track->bindBone(&skeleton, boneIndex);

            if (bundle.translation) {
                for (unsigned int i = 0; i < bundle.translation->mNumPositionKeys; ++i) {
                    const auto& k = bundle.translation->mPositionKeys[i];
                    track->addPositionKey({(float)k.mTime, {k.mValue.x, k.mValue.y, k.mValue.z}});
                }
            }

            if (bundle.rotation) {
                for (unsigned int i = 0; i < bundle.rotation->mNumRotationKeys; ++i) {
                    const auto& k = bundle.rotation->mRotationKeys[i];
                    track->addRotationKey({(float)k.mTime, glm::quat(k.mValue.w, k.mValue.x,
                                                                     k.mValue.y, k.mValue.z)});
                }
            }

            if (bundle.scaling) {
                for (unsigned int i = 0; i < bundle.scaling->mNumScalingKeys; ++i) {
                    const auto& k = bundle.scaling->mScalingKeys[i];
                    track->addScaleKey({(float)k.mTime, {k.mValue.x, k.mValue.y, k.mValue.z}});
                }
            }
        }

        std::string name =
            anim->mName.length > 0 ? anim->mName.C_Str() : "Anim_" + std::to_string(a);

        skeleton.addClip(name, clip);
    }
}

std::string dzemikk::AnimationLoader::normalizeBoneName(const std::string& name) {
    const std::string tag1 = "_$AssimpFbx$";
    const std::string tag2 = "$AssimpFbx$";

    size_t pos = name.find(tag1);
    if (pos != std::string::npos) {
        return name.substr(0, pos);
    }

    pos = name.find(tag2);
    if (pos != std::string::npos) {
        return name.substr(0, pos);
    }

    return name;
}