#pragma once

#ifndef TUL_PBL_DZEMIKK_ANIMATOR_H
#define TUL_PBL_DZEMIKK_ANIMATOR_H
#include "animation/animationstatemachine.h"
#include "ecs/component.h"

#include <string>
namespace dzemikk {

class Animator : public Component {
public:
    void update(float deltaTime);
    void play(const std::string& stateName);
    void setFloat(const std::string& name, float value);
    void setBool(const std::string& name, bool value);
private:
    AnimationStateMachine* _stateMachine = nullptr;
    float _time = 0.0f;
};

}; // namespace dzemikk
#endif // TUL_PBL_DZEMIKK_ANIMATOR_H
