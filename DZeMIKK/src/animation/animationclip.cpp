#include "animation/animationclip.h"
namespace dzemikk {
AnimationClip::AnimationClip(float durationInTicks, int ticksPerSecond) : durationInTicks(durationInTicks), ticksPerSecond(ticksPerSecond)  {}
    float AnimationClip::getDuration() const {
        return durationInTicks;
    }
    int AnimationClip::getTicksPerSecond() const {
        return ticksPerSecond;
    }
    void AnimationClip::sample(float timeInTicks, Pose& outPose) {

    }

    } // namespace dzemikk
