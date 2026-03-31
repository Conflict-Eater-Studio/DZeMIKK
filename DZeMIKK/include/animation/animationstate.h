#pragma once
#ifndef DZEMIKK_ANIMATIONSTATE_H
#define DZEMIKK_ANIMATIONSTATE_H

#include "animationclip.h"
#include "transition.h"
namespace dzemikk {
    class AnimationState {
    public:
        std::string name;
        AnimationClip* clip;

        std::vector<Transition> transitions;
    };
}
#endif
