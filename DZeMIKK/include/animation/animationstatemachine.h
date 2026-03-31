#ifndef TUL_PBL_DZEMIKK_ANIMATIONSTATEMACHINE_H
#define TUL_PBL_DZEMIKK_ANIMATIONSTATEMACHINE_H
#include "animationstate.h"

#include <map>
class AnimationStateMachine {
public:
    void update(float dt);
    AnimationState* getCurrentState() const;
private:
    std::map<std::string, AnimationState*> _states;
    AnimationState* _currentState;
};
#endif // TUL_PBL_DZEMIKK_ANIMATIONSTATEMACHINE_H
