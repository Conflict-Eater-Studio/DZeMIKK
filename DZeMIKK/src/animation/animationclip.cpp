#include "animation/animationclip.h"
#include "animation/animationtrack.h"
#include "ecs/components/transform.h"
#include <assimp/anim.h>

namespace dzemikk {
    AnimationClip::AnimationClip(float duration, float framerate) : _duration(duration), _framerate(framerate) {}

    AnimationClip::AnimationClip(const aiAnimation* animation) {
        _duration = animation->mDuration;
        _framerate = animation->mTicksPerSecond;
        _tracks.reserve(animation->mNumChannels);

        for (int i = 0; i < animation->mNumChannels; i++) {
            aiNodeAnim* track = animation->mChannels[i];
            AnimationTrack* newTrack = new AnimationTrack(track);
            _tracks.push_back(std::unique_ptr<AnimationTrack>(newTrack));
        }
    }

    float AnimationClip::getDuration() const {
        return _duration;
    }

    float AnimationClip::getFramerate() const {
        return _framerate;
    }

    AnimationTrack* AnimationClip::addTrack() {
        std::unique_ptr<AnimationTrack> track = std::make_unique<AnimationTrack>();
        AnimationTrack* ptr = track.get();
        _tracks.push_back(std::move(track));
        return ptr;
    }

    void AnimationClip::sample(float timeInSeconds) const {
        float time = timeInSeconds * getFramerate();
        float keyframe = fmod(time, getDuration());

        if (_tracks.empty()) return;

        for (auto& track : _tracks) {
            track->interpolate(keyframe);
        }
    }

}

