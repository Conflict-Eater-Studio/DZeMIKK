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
    AnimationState();
    AnimationState(const std::string& name);

    ~AnimationState() = default;

    AnimationState(const AnimationState&) = delete;
    AnimationState& operator=(const AnimationState&) = delete;

    [[nodiscard]] const std::string& getName() const noexcept;
    void setName(const std::string& name);

    [[nodiscard]] AnimationClip* getClip() const noexcept;
    void setClip(AnimationClip* clip);

    [[nodiscard]] const std::vector<Transition>& getTransitions() const noexcept;
    void setTransitions(const std::vector<Transition>& transitions);
    void addTransition(const Transition& transition);
private:
    std::string _name;
    AnimationClip* _clip = nullptr;
    std::vector<Transition> _transitions;
};

inline void to_json(nlohmann::json& json, const AnimationState& state) {
    json["name"] = state.getName();
    json["transitions"] = state.getTransitions();
}

inline void from_json(const nlohmann::json& j, AnimationState& s) {
    s.setName(j["name"]);
    s.setTransitions(j["transitions"]);
}
}
#endif