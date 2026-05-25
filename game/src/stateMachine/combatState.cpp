#include "stateMachine/combatState.h"

#include "game.h"
#include "camera/cameraController.h"

void game::CombatState::onEnter() {
    _game->getCameraController()->setMode(game::CameraController::Mode::Combat);
    //_game->enableWorldInput(false);
    //_game->enterCombatUI();
}

void game::CombatState::onExit() {
    //_game->exitCombatUI();
}
