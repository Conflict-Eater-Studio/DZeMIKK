#include "stateMachine/combatState.h"
#include "enemySystem/enemyEntity.h"
#include "map/HexGrid.h"
#include "map/HexCell.h"
#include "enemySystem/enemyManager.h"
#include "playerMovement.h"
#include "enemySystem/combatArenaBuilder.h"

#include "game.h"
#include "camera/cameraController.h"
#include <ecs/scene.h>
#include <ecs/gameobject.h>
#include <assetManager/assetHandle.h>
#include <iostream>


void game::CombatState::onEnter() {

    _game->getCameraController()->setMode(CameraController::Mode::Combat);

    _game->enableCombatUI(true);

    auto scene = _game->getCurrentScene();

    auto playerGO = scene.get()->findGameObjectByName("Player");
    player = playerGO->getComponent<game::PlayerEntity>();

    auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");
    auto* world = worldGO->getComponent<game::World>();

    auto* enemyManagerGO = scene.get()->findGameObjectByName("EnemyManager");
    auto* enemyManager = enemyManagerGO->getComponent<game::EnemyManager>();

    auto playerCell = _game->getHexGrid()->findCellByEntity(player);
    _currentEnemy = enemyManager->getEnemyByCell(playerCell.get());

    CombatArenaBuilder builder;

    auto arena = builder.build(_currentEnemy, player, _game->getHexGrid(), world);

    if (!arena.centerCell)
        return;

    auto* movement = player->getOwner()->getComponent<PlayerMovement>();

    if (movement) {
        movement->stopMovement();
    }

    player->teleportTo(arena.centerCell);
}

void game::CombatState::onExit() {

    _game->enableCombatUI(false);

    auto scene = _game->getCurrentScene();

    auto* grid = _game->getHexGrid();

    if (!grid || !_currentEnemy || !player)
        return;

    for (auto* cell : player->getTerritory()) {

        if (!cell)
            continue;

        cell->setType(HexCell::Type::Normal);
        cell->setDirty(true);
    }

    player->clearTerritory();

    const auto& enemyTerritory = _currentEnemy->getTerritory();

    for (auto* cell : enemyTerritory) {

        if (!cell)
            continue;

        cell->setType(HexCell::Type::Normal);
        cell->setDirty(true);
    }


    auto enemyCell = grid->findCellByEntity(_currentEnemy);

    if (enemyCell) {

        auto* playerMovement = player->getOwner()->getComponent<PlayerMovement>();

        if (playerMovement) {
            playerMovement->stopMovement();
        }

        player->teleportTo(enemyCell);
    }

    auto* enemyGO = _currentEnemy->getOwner();

    if (enemyGO) {
        _game->getCurrentScene().get()->destroyGameObject(enemyGO);
    }

    _currentEnemy = nullptr;
}

void game::CombatState::onUpdate(float dt) {

}