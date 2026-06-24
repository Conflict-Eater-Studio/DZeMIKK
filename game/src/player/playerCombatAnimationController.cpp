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
#include "stateMachine/cinematicState.h"

namespace game {

void PlayerCombatAnimationController::start() {
    dzemikk::UIActionRegistry::get().registerAction(
    [this](const dzemikk::UIEvent&) {
        if (canPlay) {
            playConfirmRoundAttack();
            canPlay = false;
        }
    }, "Confirm_Round_AttackAnim");
    std::string t = _playerAnimator->getCurrentState()->getName();

}

void PlayerCombatAnimationController::update(double deltaTime) {
    if (_playerAnimator) {
        if (_playerAnimator->getCurrentState()->getName() == "Attack1" && _playerAnimator->getCurrentState()->getClip()->isFinished()) {
            _playerAnimator->play("Idle");
            _playerAnimator->setBool("isFinished", true);
            canPlay = true;
        }else {
            _playerAnimator->setBool("isFinished", false);
        }
    }
    if (_playerAnimator) {
        if (_playerAnimator->getCurrentState()->getName() == "Attack2" && _playerAnimator->getCurrentState()->getClip()->isFinished()) {
            _playerAnimator->play("Idle");
            _playerAnimator->setBool("isFinished", true);
            canPlay = true;
        }else {
            _playerAnimator->setBool("isFinished", false);
        }
    }

    if (_gameStateMachine->getCurrentStateAs<ExplorationState>() || _gameStateMachine->getCurrentStateAs<CinematicState>()) {
        _playerAnimator->setBool("isFinished", true);
        canPlay = true;
        _enemyAnimator = nullptr;

    }

    if (_enemyAnimator != nullptr && _enemyAnimator->getCurrentState()->getName() == "Attack" && _enemyAnimator->getCurrentState()->getClip()->isFinished()) {
        _enemyAnimator->play("Idle");
    }

    if (_playerHealthSystem->getCurrentHealth() <= 0.0f) {
        //_playerAnimator->play("Death");
    }

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

    if (!_gameStateMachine) {
        return;
    }

    auto* combatState = _gameStateMachine->getCurrentStateAs<CombatState>();
    if (!combatState || !combatState->getCurrentEnemy()) {
        return;
    }

    auto* enemyOwner = combatState->getCurrentEnemy()->getOwner();
    if (!enemyOwner) {
        return;
    }

    _enemyAnimator = enemyOwner->getComponent<dzemikk::Animator>();
    if (_enemyAnimator) {
        _enemyAnimator->setBool("isFinished", false);
        _enemyAnimator->play("Attack");
    }

}
} // namespace game
