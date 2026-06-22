#include "player/playerCombatAnimationController.h"

#include "animation/animationclip.h"
#include "animation/animationstatemachine.h"
#include "ecs/components/animator.h"
#include "ecs/components/skinnedMeshRenderer.h"
#include "ecs/components/ui/uiActionRegistry.h"
#include "ecs/gameobject.h"
#include "enemySystem/enemyEntity.h"
#include "stateMachine/combatState.h"
#include "stateMachine/explorationState.h"

namespace game {

void PlayerCombatAnimationController::start() {
    dzemikk::UIActionRegistry::get().registerAction(
    [this](const dzemikk::UIEvent&) {
        if (canPlay) {
            playConfirmRoundAttack();
            canPlay = false;
        }
    }, "Confirm_Round_AttackAnim");
}

void PlayerCombatAnimationController::update(double deltaTime) {
    if (_playerAnimator) {
        if (_playerAnimator->getCurrentState()->getName() == "Attack1" && _playerAnimator->getCurrentState()->getClip()->isFinished()) {
            _playerAnimator->setBool("isFinished", true);
            canPlay = true;
        }else {
            _playerAnimator->setBool("isFinished", false);
        }
    }
    if (_playerAnimator) {
        if (_playerAnimator->getCurrentState()->getName() == "Attack2" && _playerAnimator->getCurrentState()->getClip()->isFinished()) {
            _playerAnimator->setBool("isFinished", true);
            canPlay = true;
        }else {
            _playerAnimator->setBool("isFinished", false);
        }
    }

    if (auto* combat = _gameStateMachine->getCurrentStateAs<ExplorationState>()) {
        _playerAnimator->setBool("isFinished", true);
        canPlay = true;
    }

    if (_enemyAnimator != nullptr && _enemyAnimator->getCurrentState()->getName() == "Attack" && _enemyAnimator->getCurrentState()->getClip()->isFinished()) {
        _enemyAnimator->play("Idle");
        _enemyAnimator = nullptr;

    }


    if (_playerHealthSystem->getCurrentHealth() <= 0.0f) {
        //_playerAnimator->play("Death");
    }



}
void PlayerCombatAnimationController::setEnemies(const std::vector<game::EnemyEntity*>& enemies) {
    _enemies = enemies;
}
void PlayerCombatAnimationController::setPlayerAnimator(dzemikk::Animator* animator) {
    _playerAnimator = animator;
}

dzemikk::Animator* PlayerCombatAnimationController::getPlayerAnimator() const {
    return _playerAnimator;
}
void PlayerCombatAnimationController::setEnemyAnimator(dzemikk::Animator* animator) {
    _enemyAnimator = animator;
}
void PlayerCombatAnimationController::setHealthSystem(HealthSystem* healthSystem) {
    _playerHealthSystem = healthSystem;
}
HealthSystem* PlayerCombatAnimationController::getHealthSystem() const {
    return _playerHealthSystem;
}
void PlayerCombatAnimationController::setGameStateMachine(GameStateMachine* gameStateMachine) {
    _gameStateMachine = gameStateMachine;
}
GameStateMachine* PlayerCombatAnimationController::getGameStateMachine() const {
    return _gameStateMachine;
}

void PlayerCombatAnimationController::playConfirmRoundAttack() {
    if (_playerAnimator) {
        if (animationAttackOne) {
            _playerAnimator->play("Attack1");
            animationAttackOne = false;
        }else {
            _playerAnimator->play("Attack2");
            animationAttackOne = true;

        }
    }
    if (_enemyAnimator) {
        _enemyAnimator->play("Attack");
        return;
    }
    for (auto* enemy : _enemies) {
        if (enemy->getOwner()->getName() == _gameStateMachine->getCurrentStateAs<CombatState>()->getCurrentEnemy()->getOwner()->getName()){
            _enemyAnimator = enemy->getOwner()->getComponent<dzemikk::Animator>();
            _enemyAnimator->setBool("isFinished",0);
            _enemyAnimator->play("Attack");
        }
    }

}
} // namespace game
