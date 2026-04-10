#include "animation/animationclip.h"

#include "animation/animationtrack.h"
#include "ecs/components/transform.h"
#include "spdlog/spdlog.h"
namespace dzemikk {
    AnimationClip::AnimationClip(int duration, int framerate) : _duration(duration), _framerate(framerate) {}

    float AnimationClip::getDuration() const {
        return _duration;
    }

    int AnimationClip::getFramerate() const {
        return _framerate;
    }

    void AnimationClip::addTrack(AnimationTrack* track) {
        _tracks.push_back(track);
    }

    void AnimationClip::sample(float currentTime) const {
        float time = currentTime * getFramerate(); // seconds in animation clip timeline
        float keyframe = fmod(time, getDuration());

        if (_tracks.empty()) return;

        for (auto& track : _tracks) {
            track->interpolate(keyframe);
        }
    }

}

