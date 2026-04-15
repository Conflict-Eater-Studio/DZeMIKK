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
        ~AnimationState() = default;
        [[nodiscard]] const std::string& getName() const noexcept;
        void setName(const std::string& name);

        [[nodiscard]] AnimationClip* getClip() const noexcept;
        void setClip(AnimationClip* clip);

        [[nodiscard]] const std::vector<Transition>& getTransitions() const noexcept;
        void addTransition(const Transition& transition);
        void update(float deltaTime);
    private:
        std::string _name;
        AnimationClip* _clip = nullptr;
        std::vector<Transition> _transitions;
    };
}
#endif
