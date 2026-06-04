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
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[AnimationClip] AnimationClip has no tracks!");
#endif
        return;
    }

    float keyframe = time;
    if (_loop) {
        keyframe = fmod(keyframe, _durationInTicks);
    } else if (_durationInTicks > 0.0f && time > _durationInTicks) {
        keyframe = _durationInTicks;
    }

    for (auto& track : _tracks) {
        track->apply(keyframe);
    }
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
void AnimationClip::setName(const std::string& name) {
    _nameInSkeleton = name;
}
std::string AnimationClip::getName() {
    return _nameInSkeleton;
}
}

