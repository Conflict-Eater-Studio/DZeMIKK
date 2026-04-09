#pragma once
#ifndef DZEMIKK_ANIMATOR_H
#define DZEMIKK_ANIMATOR_H
#include "animation/animationstatemachine.h"
#include "ecs/component.h"

#include <memory>
#include <string>
#include <string_view>

namespace dzemikk {
    class Animator : public Component {
    public:
        void update(float deltaTime);
        void play(const std::string& stateName);

        void setFloat(std::string_view name, float value);
        void setBool(std::string_view name, bool value);
        void setInt(std::string_view name, int value);

        void setStateMachine(const std::shared_ptr<AnimationStateMachine>& stateMachine);
        [[nodiscard]] std::shared_ptr<AnimationStateMachine> getStateMachine() const noexcept;
    private:
        std::shared_ptr<AnimationStateMachine> _stateMachine = nullptr;
        float _currentTime = 0.0f;
    };
}
#endif