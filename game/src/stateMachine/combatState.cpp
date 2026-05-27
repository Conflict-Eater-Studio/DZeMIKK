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
#include <iostream>


void game::CombatState::onEnter() {
    _game->getCameraController()->setMode(game::CameraController::Mode::Combat);
    _game->enableCombatUI(true);

    auto scene = _game->getCurrentScene();

    auto playerGO = scene.get()->findGameObjectByName("Player");
    auto* player = playerGO->getComponent<game::PlayerEntity>();

    if (!player) {
        spdlog::info("Player not found in scene!");
        return;
    }

    auto* grid = _game->getHexGrid();
    if (!grid) {
        spdlog::info("HexGrid not found!");
        return;
    }

    auto playerCell = grid->findCellByEntity(player);
    if (!playerCell) {
        spdlog::info("Player is not assigned to any HexCell!");
        return;
    }

    if (playerCell->getType() != HexCell::Type::EnemyBattleHex)
        return;

    auto* enemyManagerGO = scene.get()->findGameObjectByName("EnemyManager");

    auto* enemyManager = enemyManagerGO->getComponent<game::EnemyManager>();

    auto enemy = enemyManager->getEnemyByCell(playerCell.get());

    if (!enemy)
        return;

    HexCoord center = playerCell->getCoord();
    center += HexCoord::dir(HexCoord::Direction::R0) * 3;

    auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");
    auto* world = worldGO->getComponent<game::World>();

    HexChunk* chunk = nullptr;
    auto& chunks = world->getGrid()->getChunks();

    for (auto& [id, chunkPtr] : chunks) {
        if (chunkPtr->contains(center)) {
            chunk = chunkPtr.get();
        }
    }

    std::vector<HexCoord> offsets;
    offsets.reserve(enemy->getTerritory().size());

    for (auto* cell : enemy->getTerritory()) {
        offsets.push_back(cell->getCoord());
    }

    assignPlayerTerritory(player, chunk, grid, offsets, center);

    spdlog::info("Combat layout initialized vs enemy");
}

void game::CombatState::onExit() {
    _game->enableCombatUI(false);
}

void game::CombatState::onUpdate(float dt) {

}

void game::CombatState::assignPlayerTerritory(game::PlayerEntity* player, HexChunk* chunk,
                                              HexGrid* grid, const std::vector<HexCoord>& offsets,
                                              const HexCoord& center) {
    for (const auto& offset : offsets) {

        HexCoord coord = center + offset;

        auto targetCell = grid->getCell(coord);

        auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");
        auto* world = worldGO->getComponent<game::World>();
        bool hasVisual = world->hasHexVisual(coord);

        if (!targetCell || !hasVisual) {
            auto newCell = std::make_shared<HexCell>(coord, HexCell::State::Empty,
                                                     HexCell::Type::PlayerBattleHex,
                                                     HexCell::GenState::Normal);

            if (!chunk) {
                chunk = grid->getChunks().begin()->second.get();
            }

            chunk->insertCell(coord, newCell);

            targetCell = newCell;

            world->ensureHexExists(newCell);
        }

        targetCell->setType(HexCell::Type::PlayerBattleHex);
        targetCell->setEntity(player);
        targetCell->setDirty(true);

        player->addTerritoryCell(targetCell.get());
    }
}