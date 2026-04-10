#pragma once
#ifndef DZEMIKK_ANIMATIONMODULE_H
#define DZEMIKK_ANIMATIONMODULE_H

#include <vector>
#include "core/iEngineModule.h"

namespace dzemikk {
    class Animator;
    class AnimationModule : public IEngineModule  {
    public:
        AnimationModule() = default;
        void update(float deltaTime) const;
        void Initialize() override;
        void UnInitialize() override;
    private:
        std::vector<Animator*> _animators;
    };
}
#endif
