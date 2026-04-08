#pragma once
#ifndef DZEMIKK_ANIMATIONCLIP_H
#define DZEMIKK_ANIMATIONCLIP_H
#include "pose.h"
#include <vector>

namespace dzemikk {
    class AnimationCurve;
    class AnimationClip {
    public:
        AnimationClip(int length, int framerate);
        int getLength() const;
        int getFramerate() const;
        void clear();
        void addPose(const Pose& pose);
        Pose sample(float time) const;
        //std::vector<float> sample(float timeInTicks) const; // TODO: Blending shape
    private:
        //std::vector<AnimationCurve> _curves;
        std::vector<Pose> _poses;
        int _length = 0;
        int _framerate = 0;

    };
};
#endif
