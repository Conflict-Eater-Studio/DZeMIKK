#pragma once
#ifndef DZEMIKK_ANIMATIONSTATE_H
#define DZEMIKK_ANIMATIONSTATE_H

#include <memory>
#include <string>
#include <vector>

struct Transition;
namespace dzemikk {
    class AnimationClip;
    class AnimationState {
    public:
        std::string name;
        AnimationClip* clip;

        std::vector<Transition> transitions;
    };
}
#endif
