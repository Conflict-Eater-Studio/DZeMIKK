#include "animation/animationclip.h"
#include "animation/pose.h"

namespace dzemikk {
AnimationClip::AnimationClip(int frames, int framerate) : _length(frames), _framerate(framerate)  {}
    int AnimationClip::getLength() const {
        return _length;
    }
    int AnimationClip::getFramerate() const {
        return _framerate;
    }
    void AnimationClip::addPose(const Pose& pose) {
        _poses.push_back(pose);
    }
    void AnimationClip::sample(float timeInTicks, Pose& outPose) {

    }

    } // namespace dzemikk
