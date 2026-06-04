#pragma once

#include "animation/animationstate.h"
#include "animation/animationstatemachine.h"
#include "ecs/components/animator.h"

#include <array>

namespace editor {

class AnimatorStateMachinePanel {
  public:
    void open(dzemikk::Animator* animator);
    void draw(dzemikk::Animator* animator);

    bool isOpen() const {
        return _open;
    }

  private:
    void drawStates(dzemikk::AnimationStateMachine* sm);
    void drawSelectedState();
    void drawAddState(dzemikk::AnimationStateMachine* sm);
    void drawTransitions();
    void cacheStateNames(dzemikk::AnimationStateMachine* sm);

  private:
    bool _open = false;

    dzemikk::Animator* _animator = nullptr;
    dzemikk::AnimationState* _selectedState = nullptr;

    // UI buffers
    std::array<char, 64> _newStateName{};
    int _targetStateIndex = 0;
    std::vector<std::string> _cachedStateNames;
    float _transitionDuration = 0.25f;

    std::array<char, 64> _condParam{};
    int _condOp = 0;
    float _condValue = 0.0f;
    int _condType = 0;
};

} // namespace editor