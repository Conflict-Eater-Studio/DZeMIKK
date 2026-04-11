#include "animation/animationclip.h"

#include "animation/IAnimationTrack.h"
#include "animation/animationtrack.h"
#include "animation/floattrack.h"
#include "ecs/components/transform.h"

#include <assimp/anim.h>

namespace dzemikk {
    AnimationClip::AnimationClip(float duration, float framerate) : _duration(duration), _framerate(framerate) {}

    float AnimationClip::getDuration() const {
        return _duration;
    }

    float AnimationClip::getFramerate() const {
        return _framerate;
    }

    FloatTrack* AnimationClip::addFloatTrack() {
        std::unique_ptr<FloatTrack> track = std::make_unique<FloatTrack>();
        FloatTrack* ptr = track.get();
        _tracks.push_back(std::move(track));
        return ptr;
    }

    void AnimationClip::apply(float timeInSeconds) const {
        float time = timeInSeconds * getFramerate();
        float keyframe = fmod(time, getDuration());

        if (_tracks.empty()) return;

        for (auto& track : _tracks) {
            track->apply(keyframe);
        }
    }

}

