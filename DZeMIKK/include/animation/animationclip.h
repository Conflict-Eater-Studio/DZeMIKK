#pragma once
#ifndef DZEMIKK_ANIMATIONCLIP_H
#define DZEMIKK_ANIMATIONCLIP_H
#include "animation/animationcurve.h"
#include <vector>

namespace dzemikk {
class Transform;
class AnimationClip {
    public:
        // If you reading this, imagine line below doesn't exist
        Transform* transform;
        static const int DEFAULT_FRAMERATE = 30;
        static const int DEFAULT_LENGTH = 100;

        AnimationClip() = default;
        AnimationClip(int duration, int framerate);
        float getDuration() const;
        int getFramerate() const;
        void addCurve(const AnimationCurve& curve);
        void sample(float timeInTicks) const; // TODO: Blending shape
    private:
        std::vector<AnimationCurve> _curves;
        int _duration = 0;
        int _framerate = 0;
    };
};
#endif
