#pragma once
#ifndef DZEMIKK_ANIMATIONCLIP_H
#define DZEMIKK_ANIMATIONCLIP_H

namespace dzemikk {
    class Pose;
    class AnimationClip {
    public:
        AnimationClip(float durationInTicks, int ticksPerSecond);
        float getDuration() const;
        int getTicksPerSecond() const;
        void sample(float timeInTicks, Pose& outPose);
    private:
        float durationInTicks = 0.0f;
        int ticksPerSecond = 0;

    };
};
#endif
