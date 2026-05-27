#include "stateMachine/combatState.h"
#include "enemySystem/enemyEntity.h"
#include "map/HexGrid.h"
#include "map/HexCell.h"
#include "enemySystem/enemyManager.h"
#include "playerMovement.h"

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

    if (!player)
        return;

    auto* grid = _game->getHexGrid();

    if (!grid)
        return;

    auto playerCell = grid->findCellByEntity(player);

    if (!playerCell)
        return;

    if (playerCell->getType() != HexCell::Type::EnemyBattleHex)
        return;

    auto* enemyManagerGO = scene.get()->findGameObjectByName("EnemyManager");
    auto* enemyManager = enemyManagerGO->getComponent<game::EnemyManager>();

    _currentEnemy = enemyManager->getEnemyByCell(playerCell.get());

    if (!_currentEnemy)
        return;

    const auto& territory = _currentEnemy->getTerritory();

    if (territory.empty())
        return;

    std::vector<HexCoord> copiedCoords;
    copiedCoords.reserve(territory.size());

    for (auto* cell : territory) {
        if (cell)
            copiedCoords.push_back(cell->getCoord());
    }

    std::unordered_set<HexCoord> blocked;

    for (auto* cell : territory) {
        if (!cell)
            continue;

        HexCoord c = cell->getCoord();

        blocked.insert(c);

        for (const auto& n : HexCoord::getNeighbors(c)) {
            blocked.insert(n);
        }
    }

    bool overlaps = true;

    HexCoord shiftLeft(-1, 1);

    while (overlaps) {

        overlaps = false;

        for (const auto& coord : copiedCoords) {

            if (blocked.contains(coord)) {
                overlaps = true;
                break;
            }
        }

        if (overlaps) {
            for (auto& coord : copiedCoords) {
                coord += shiftLeft;
            }
        }
    }

    auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");
    auto* world = worldGO->getComponent<game::World>();

    HexChunk* chunk = nullptr;
    auto& chunks = world->getGrid()->getChunks();

    if (!chunks.empty()) {
        chunk = chunks.begin()->second.get();
    }

    if (!chunk)
        return;

    for (const auto& coord : copiedCoords) {

        auto cell = grid->getCell(coord);
        bool hasVisual = world->hasHexVisual(coord);

        if (!cell || !hasVisual) {

            auto newCell = std::make_shared<HexCell>(
                coord, HexCell::State::Empty, HexCell::Type::PlayerBattleHex, HexCell::GenState::Normal);

            chunk->insertCell(coord, newCell);
            world->ensureHexExists(newCell);

            cell = newCell;
        }

        cell->setType(HexCell::Type::PlayerBattleHex);
        player->addTerritoryCell(cell.get());
        cell->setDirty(true);
    }

    int sumQ = 0;
    int sumR = 0;

    for (const auto& coord : copiedCoords) {
        sumQ += coord.q();
        sumR += coord.r();
    }

    HexCoord centerCoord(sumQ / static_cast<int>(copiedCoords.size()),
                         sumR / static_cast<int>(copiedCoords.size()));

    auto centerCell = grid->getCell(centerCoord);

    bool hasVisual = world->hasHexVisual(centerCoord);

    if (!centerCell || !hasVisual) {

        auto newCenterCell =
            std::make_shared<HexCell>(centerCoord, HexCell::State::Empty,
                                      HexCell::Type::Normal, HexCell::GenState::Normal);

        chunk->insertCell(centerCoord, newCenterCell);

        world->ensureHexExists(newCenterCell);

        centerCell = newCenterCell;
    }

    centerCell->setDirty(true);

    auto* playerMovement = player->getOwner()->getComponent<game::PlayerMovement>();
    playerMovement->stopMovement();
    player->teleportTo(centerCell);
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