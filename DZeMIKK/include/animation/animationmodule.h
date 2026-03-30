#pragma once
#ifndef TUL_PBL_DZEMIKK_ANIMATIONMODULE_H
#define TUL_PBL_DZEMIKK_ANIMATIONMODULE_H

#include "core/iEngineModule.h"
#include "ecs/components/animator.h"

#include <memory>
#include <vector>
namespace dzemikk {
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
#endif // TUL_PBL_DZEMIKK_ANIMATIONMODULE_H
