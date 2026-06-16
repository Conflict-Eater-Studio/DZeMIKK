#include "spriteAnimation/spriteAnimationModule.h"

#include "ecs/componentRegistry.h"
#include "ecs/components/ui/spriteAnimation.h"

namespace dzemikk {
void SpriteAnimationModule::initialize() {}

void SpriteAnimationModule::uninitialize() {
    _activeAnimations.clear();
}

void SpriteAnimationModule::update(float deltaTime) {
    _activeAnimations.clear();
    ComponentRegistry::get().getEnabledComponents<SpriteAnimation>(_activeAnimations);

    for (auto* anim : _activeAnimations) {
        anim->update(deltaTime);
    }
}
} // namespace dzemikk
