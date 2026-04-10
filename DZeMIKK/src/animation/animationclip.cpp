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

