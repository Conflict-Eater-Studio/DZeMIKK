#include "animation/animationclip.h"
#include "animation/pose.h"

namespace dzemikk {
AnimationClip::AnimationClip(int frames, int framerate) : _length(frames), _framerate(framerate) {
    _poses.reserve(frames);
    }
    int AnimationClip::getLength() const {
        return _length;
    }
    int AnimationClip::getFramerate() const {
        return _framerate;
    }
    void AnimationClip::addPose(const Pose& pose) {
        _poses.push_back(pose);
    }
    Pose AnimationClip::sample(float timeInTicks) {
        int frame = (int)(timeInTicks * _framerate);
        return _poses[frame];
    }

    } // namespace dzemikk
