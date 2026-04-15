#pragma once
#ifndef DZEMIKK_ANIMATIONMODULE_H
#define DZEMIKK_ANIMATIONMODULE_H

#include <vector>
#include <algorithm>
#include "core/iEngineModule.h"

namespace dzemikk {

class Animator;

/**
 * @brief Centralized system module responsible for updating all active
 * Animator components in the engine.
 */
class AnimationModule : public IEngineModule {
public:
    AnimationModule() = default;
    ~AnimationModule() override = default;

    // --- IEngineModule Implementation ---
    void Initialize() override;
    void UnInitialize() override;

    /**
     * @brief Updates all registered animators.
     * @param deltaTime Time elapsed since the last frame.
     */
    void update(float deltaTime);

    /**
     * @brief Registers an animator to be updated every frame.
     * Call this when a new Animator component is created.
     */
    void registerAnimator(Animator* animator);

    /**
     * @brief Unregisters an animator.
     * CRITICAL: Call this before the Animator is destroyed to prevent dangling pointers.
     */
    void unregisterAnimator(const Animator* animator);

private:
    /**
     * @brief Animators pointers
     */
    std::vector<Animator*> _animators;
};

} // namespace dzemikk

#endif // DZEMIKK_ANIMATIONMODULE_H