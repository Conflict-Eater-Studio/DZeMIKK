#include "player/playerCombatAnimationController.h"

#include "animation/animationclip.h"
#include "animation/animationstatemachine.h"
#include "ecs/components/animator.h"
#include "ecs/components/ui/uiActionRegistry.h"

namespace game {

void PlayerCombatAnimationController::start() {
    dzemikk::UIActionRegistry::get().registerAction(
    [this](const dzemikk::UIEvent&) {
        playConfirmRoundAttack();
    }, "Confirm_Round_AttackAnim");
}
void PlayerCombatAnimationController::update(double deltaTime) {
    if (_playerAnimator == nullptr) return;

    if (_playerAnimator->getCurrentState()->getName() == "Attack1" && _playerAnimator->getCurrentState()->getClip()->isFinished()) {
        _playerAnimator->setBool("isFinished", true);
    }else {
        _playerAnimator->setBool("isFinished", false);
    }


}
void PlayerCombatAnimationController::setPlayerAnimator(dzemikk::Animator* animator) {
    _playerAnimator = animator;
}

dzemikk::Animator* PlayerCombatAnimationController::getPlayerAnimator() const {
    return _playerAnimator;
}
void PlayerCombatAnimationController::setHealthSystem(HealthSystem* healthSystem) {
    _playerHealthSystem = healthSystem;
}
HealthSystem* PlayerCombatAnimationController::getHealthSystem() const {
    return _playerHealthSystem;
}

void PlayerCombatAnimationController::playConfirmRoundAttack() const {
    if (!_playerAnimator) {
        return;
    }

    _playerAnimator->play("Attack1");
}

} // namespace game
