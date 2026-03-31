#ifndef TUL_PBL_DZEMIKK_ANIMATIONSTATE_H
#define TUL_PBL_DZEMIKK_ANIMATIONSTATE_H
#include "animationclip.h"
#include "transition.h"
class AnimationState {
public:
    std::string name;
    dzemikk::AnimationClip* clip;

    std::vector<Transition> transitions;
};
#endif // TUL_PBL_DZEMIKK_ANIMATIONSTATE_H
