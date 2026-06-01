#include "stateMachine/combatState.h"
#include "enemySystem/enemyEntity.h"
#include "map/HexGrid.h"
#include "map/HexCell.h"
#include "enemySystem/enemyManager.h"
#include "playerMovement.h"
#include "enemySystem/combatArenaBuilder.h"
#include "player/playerPatternComponent.h"

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
    _player = playerGO->getComponent<game::PlayerEntity>();

    auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");
    auto* world = worldGO->getComponent<game::World>();

    auto* enemyManagerGO = scene.get()->findGameObjectByName("EnemyManager");
    auto* enemyManager = enemyManagerGO->getComponent<game::EnemyManager>();

    auto playerCell = _game->getHexGrid()->findCellByEntity(_player);
    _currentEnemy = enemyManager->getEnemyByCell(playerCell.get());

    CombatArenaBuilder builder;

    auto arena = builder.build(_currentEnemy, _player, _game->getHexGrid(), world);

    if (!arena.centerCell)
        return;

    auto* movement = _player->getOwner()->getComponent<PlayerMovement>();

    if (movement) {
        movement->stopMovement();
    }

    _player->teleportTo(arena.centerCell);

    _playerPatternComponent = playerGO->getComponent<PlayerPatternComponent>();
}

void game::CombatState::onExit() {

    _game->enableCombatUI(false);

    auto scene = _game->getCurrentScene();

    auto* grid = _game->getHexGrid();

    if (!grid || !_currentEnemy || !_player)
        return;

    for (auto* cell : _player->getTerritory()) {

        if (!cell)
            continue;

        cell->setType(HexCell::Type::Normal);
        cell->setDirty(true);
    }

    _player->clearTerritory();
    _playerPatternComponent->clearActivePattern();
    _playerPatternComponent->clearPlacedPatterns();
    _playerPatternComponent->clearPreview();

    const auto& enemyTerritory = _currentEnemy->getTerritory();

    for (auto* cell : enemyTerritory) {

        if (!cell)
            continue;

        cell->setType(HexCell::Type::Normal);
        cell->setDirty(true);
    }


    auto enemyCell = grid->findCellByEntity(_currentEnemy);

    if (enemyCell) {

        auto* playerMovement = _player->getOwner()->getComponent<PlayerMovement>();

        if (playerMovement) {
            playerMovement->stopMovement();
        }

        _player->teleportTo(enemyCell);
    }

    auto* enemyGO = _currentEnemy->getOwner();

    if (enemyGO) {
        _game->getCurrentScene().get()->destroyGameObject(enemyGO);
    }

    _currentEnemy = nullptr;
}

void game::CombatState::onUpdate(float dt) {

}