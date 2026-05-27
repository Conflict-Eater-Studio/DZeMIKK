#include "stateMachine/combatState.h"
#include "enemySystem/enemyEntity.h"
#include "map/HexGrid.h"
#include "map/HexCell.h"
#include "enemySystem/enemyManager.h"

#include "game.h"
#include "camera/cameraController.h"
#include <ecs/scene.h>
#include <ecs/gameobject.h>
#include <assetManager/assetHandle.h>

void game::CombatState::onEnter() {
    _game->getCameraController()->setMode(game::CameraController::Mode::Combat);
    _game->enableCombatUI(true);

    auto scene = _game->getCurrentScene(); 

    auto playerGO = scene.get()->findGameObjectByName("Player");
    game::PlayerEntity* player = playerGO->getComponent<game::PlayerEntity>();

    if (!player) {
        spdlog::info("Player not found in scene!");
        return;
    }

    game::HexGrid* grid = _game->getHexGrid();

    if (!grid) {
        spdlog::info("HexGrid not found!");
        return;
    }

    auto playerCell = grid->findCellByEntity(player);

    if (!playerCell) {
        spdlog::info("Player is not assigned to any HexCell!");
        return;
    }

    auto coord = playerCell->getCoord();

    if (playerCell->getType() == HexCell::Type::EnemyBattleHex) {
        auto* enemyManagerGO = _game->getCurrentScene().get()->findGameObjectByName("EnemyManager");

        auto* enemyManager = enemyManagerGO->getComponent<game::EnemyManager>();
        auto enemy = enemyManager->getEnemyByCell(playerCell.get());

        if (enemy) {
         ///combat enemy 
        }
    }
}

void game::CombatState::onExit() {
    _game->enableCombatUI(false);
}

void game::CombatState::onUpdate(float dt) {

}
