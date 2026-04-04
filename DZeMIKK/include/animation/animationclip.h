#pragma once
#ifndef DZEMIKK_ANIMATIONCLIP_H
#define DZEMIKK_ANIMATIONCLIP_H
#include "animation/pose.h"

#include <vector>

namespace dzemikk {
    class AnimationClip {
    public:
        AnimationClip(int length, int framerate);
        int getLength() const;
        int getFramerate() const;
        const std::vector<Pose>& getPoses() const;
        void setPoses(const std::vector<Pose>& poses);
        void addPose(const Pose& pose);
        void clear();
        Pose sample(float timeInTicks); // TODO: Blending shape
    private:
        std::vector<Pose> _poses;
        int _length = 0;
        int _framerate = 0;

    };
};
#endif
