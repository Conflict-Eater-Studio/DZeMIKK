#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

#include <assimp/anim.h>

#include "animation/animationclip.h"
#include "animation/floattrack.h"
#include "animation/vectortrack.h"
#include "animation/quaterniontrack.h"
#include "animation/boneTrack.h"

namespace dzemikk {
AnimationClip::AnimationClip(float durationInTicks, float tickPerSecond) : _durationInTicks(durationInTicks), _ticksPerSecond(tickPerSecond) {}

float AnimationClip::getTickDuration() const {
    return _durationInTicks;
}

float AnimationClip::getTickrate() const {
    return _ticksPerSecond;
}
const std::vector<std::unique_ptr<IAnimationTrack>>& AnimationClip::getTracks() const {
    return _tracks;
}

FloatTrack* AnimationClip::addFloatTrack() {
    std::unique_ptr<FloatTrack> track = std::make_unique<FloatTrack>();
    FloatTrack* ptr = track.get();
    _tracks.push_back(std::move(track));
    return ptr;
}
VectorTrack* AnimationClip::addVectorTrack() {
    std::unique_ptr<VectorTrack> track = std::make_unique<VectorTrack>();
    VectorTrack* ptr = track.get();
    _tracks.push_back(std::move(track));
    return ptr;
}
QuaternionTrack* AnimationClip::addQuaternionTrack() {
    std::unique_ptr<QuaternionTrack> track = std::make_unique<QuaternionTrack>();
    QuaternionTrack* ptr = track.get();
    _tracks.push_back(std::move(track));
    return ptr;
}

BoneTrack* AnimationClip::addBoneTrack() {
    auto track = std::make_unique<BoneTrack>();
    auto ptr = track.get();
    _tracks.push_back(std::move(track));
    return ptr;
}

void AnimationClip::apply(float timeInSeconds) const {
    float time = timeInSeconds * _ticksPerSecond;

    if (_tracks.empty()) {
        return;
    }

    float keyframe = time;
    if (_loop) {
        keyframe = fmod(time, _durationInTicks);
    } else if (_durationInTicks > 0.0f && time > _durationInTicks) {
        keyframe = _durationInTicks;
    }

    for (auto& track : _tracks) {
        track->apply(keyframe);
    }
}

std::shared_ptr<AnimationClip> AnimationClip::fromAssimp(aiAnimation* animation) {
    if (!animation) return nullptr;

    std::shared_ptr<AnimationClip> clip = std::make_shared<AnimationClip>();

    clip->_ticksPerSecond = (animation->mTicksPerSecond != 0.0) ? animation->mTicksPerSecond : 24.0f;

    clip->_durationInTicks = animation->mDuration;

    for (unsigned int i = 0; i < animation->mNumChannels; ++i) {
        aiNodeAnim* channel = animation->mChannels[i];
        std::string targetBoneName = channel->mNodeName.C_Str();

        if (channel->mNumPositionKeys > 0) {
            VectorTrack* posTrack = clip->addVectorTrack();

            // TODO: Get Transform from boneName and bind
            // posTrack->setTargetName(targetBoneName);

            for (unsigned int j = 0; j < channel->mNumPositionKeys; ++j) {
                const aiVectorKey& key = channel->mPositionKeys[j];

                auto time = static_cast<float>(key.mTime);
                glm::vec3 value(key.mValue.x, key.mValue.y, key.mValue.z);

                VectorPropertyKey keyFrame = {time, value};
                posTrack->addKey(keyFrame);
            }
        }

        if (channel->mNumRotationKeys > 0) {
            QuaternionTrack* rotTrack = clip->addQuaternionTrack();

            for (unsigned int j = 0; j < channel->mNumRotationKeys; ++j) {
                const aiQuatKey& key = channel->mRotationKeys[j];
                auto time = static_cast<float>(key.mTime);

                glm::quat value(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z);

                QuaternionPropertyKey keyFrame = {time, value};
                rotTrack->addKey(keyFrame);
            }
        }

        // --- EXTRACT SCALES ---
        if (channel->mNumScalingKeys > 0) {
            VectorTrack* scaleTrack = clip->addVectorTrack();

            for (unsigned int j = 0; j < channel->mNumScalingKeys; ++j) {
                const aiVectorKey& key = channel->mScalingKeys[j];
                auto time = static_cast<float>(key.mTime);
                glm::vec3 value(key.mValue.x, key.mValue.y, key.mValue.z);

                VectorPropertyKey keyFrame = {time, value};
                scaleTrack->addKey(keyFrame);
            }
        }
    }

#if DZEMIKK_DEV_TOOLS
    spdlog::info("AnimationClip loaded with {} tracks", clip->_tracks.size());
    spdlog::info("Duration: {}, Framerate: {}", clip->_durationInTicks, clip->_ticksPerSecond);
#endif

    return clip;
}
void AnimationClip::setLoop(bool loop) {
    _loop = loop;
}
void AnimationClip::setDuration(float duration) {
    _durationInTicks = duration;
}
void AnimationClip::setTickrate(float tickrate) {
    _ticksPerSecond = tickrate;
}
void AnimationClip::setTracks(std::vector<std::unique_ptr<IAnimationTrack>> tracks) {
    _tracks = std::move(tracks);
}
bool AnimationClip::isLoop() const {
    return _loop;
}
}

