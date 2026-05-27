#include "stateMachine/combatState.h"

#include "game.h"
#include "camera/cameraController.h"

void game::CombatState::onEnter() {
    _game->getCameraController()->setMode(game::CameraController::Mode::Combat);
    _game->enableCombatUI(true);
}

void game::CombatState::onExit() {
    _game->enableCombatUI(false);
}
