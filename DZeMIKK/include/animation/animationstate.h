#pragma once
#ifndef DZEMIKK_ANIMATIONSTATE_H
#define DZEMIKK_ANIMATIONSTATE_H

#include <string>
#include <vector>
#include "animation/transition.h"

namespace dzemikk {
    class AnimationClip;
    class AnimationState {
    public:
        AnimationState(std::string name);
        ~AnimationState();
        const std::string& getName() const;
        void setName(const std::string& name);
        AnimationClip* getClip() const;
        void setClip(AnimationClip* clip);
        const std::vector<Transition>& getTransitions() const;
        void addTransition(const Transition& transition);
        void update(float deltaTime);
    private:
        std::string _name;
        AnimationClip* _clip = nullptr;
        std::vector<Transition> _transitions;
    };
}
#endif
