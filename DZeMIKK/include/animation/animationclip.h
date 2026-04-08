#pragma once
#ifndef DZEMIKK_ANIMATIONCLIP_H
#define DZEMIKK_ANIMATIONCLIP_H
#include "animation/animationcurve.h"
#include <vector>

namespace dzemikk {
    class AnimationClip {
    public:
        AnimationClip(int length, int framerate);
        int getLength() const;
        int getFramerate() const;
        void clear();
        void addCurve(const AnimationCurve& curve);
        AnimationCurve getCurve(int index) const;
        void setCurve(int index, const AnimationCurve& curve);
        float sample(float timeInTicks) const; // TODO: Blending shape
    private:
        std::vector<AnimationCurve> _curves;
        int _length = 0;
        int _framerate = 0;
    };
};
#endif
