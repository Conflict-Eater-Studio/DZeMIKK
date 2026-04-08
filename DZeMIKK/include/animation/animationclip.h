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
        AnimationClip(int length, int framerate);
        int getLength() const;
        int getFramerate() const;
        void clear();
        void addCurve(const AnimationCurve& curve);
        AnimationCurve getCurve(int index) const;
        void setCurve(int index, const AnimationCurve& curve);
        void sample(float timeInTicks) const; // TODO: Blending shape
        void update(float deltaTime);
    private:
        std::vector<AnimationCurve> _curves;
        float _time = 0.0f;
        int _length = 0;
        int _framerate = 0;
    };
};
#endif
