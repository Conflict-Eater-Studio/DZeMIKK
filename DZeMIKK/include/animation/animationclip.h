#pragma once
#ifndef DZEMIKK_ANIMATIONCLIP_H
#define DZEMIKK_ANIMATIONCLIP_H

namespace dzemikk {
    class Pose;
    class AnimationClip {
    public:
        float durationInTicks;
        float ticksPerSecond;

        void sample(float timeInTicks, Pose& outPose);

    };
};
#endif
