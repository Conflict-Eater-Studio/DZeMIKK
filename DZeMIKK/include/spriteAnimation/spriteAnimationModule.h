#ifndef DZEMIKK_SPRITEANIMATIONMODULE_H
#define DZEMIKK_SPRITEANIMATIONMODULE_H

#include "core/iEngineModule.h"

#include <vector>

namespace dzemikk {
class SpriteAnimation;
class SpriteAnimationModule : public IEngineModule {
  public:
    SpriteAnimationModule() = default;
    SpriteAnimationModule(const SpriteAnimationModule&) = delete;
    SpriteAnimationModule& operator=(const SpriteAnimationModule&) = delete;
    SpriteAnimationModule(SpriteAnimationModule&&) = delete;
    SpriteAnimationModule& operator=(SpriteAnimationModule&&) = delete;
    ~SpriteAnimationModule() override = default;

    void initialize() override;
    void uninitialize() override;

    void update(float deltaTime);
    // void registerAnimator();
    // void unregisterAnimator();

  private:
    std::vector<SpriteAnimation*> _activeAnimations;
};
} // namespace dzemikk
#endif
