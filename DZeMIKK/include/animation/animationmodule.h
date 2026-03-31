#pragma once
#ifndef DZEMIKK_ANIMATIONMODULE_H
#define DZEMIKK_ANIMATIONMODULE_H

#include "core/iEngineModule.h"

#include <memory>
#include <vector>
namespace dzemikk {
    class Animator;
    class AnimationModule : public IEngineModule  {
    private:
        std::vector<std::shared_ptr<Animator>> _animators;
    public:
        AnimationModule() = default;
        void update(float deltaTime) const;
        void Initialize() override;
        void UnInitialize() override;
    };
}
#endif
