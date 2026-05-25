#include "stateMachine/explorationState.h"

#include "game.h"
#include "camera/cameraController.h"

void game::ExplorationState::onEnter() {
    _game->getCameraController()->setMode(game::CameraController::Mode::Exploration);
    //_game->enableWorldInput(true);
}

void game::ExplorationState::onExit() {
    //_game->enableWorldInput(false);
}
