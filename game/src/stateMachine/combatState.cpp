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
#include <ecs/components/meshRenderer.h>
#include <assetManager/assetHandle.h>
#include <iostream>


void game::CombatState::onEnter() {

    _phase = CombatPhase::PreparingBoard;

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

    _endTurnListenerId = _game->getEngine()->getInput()->OnKeyPressed.addListener(
        [this](dzemikk::KeyPressedEvent& e) {
            if (e.GetKeyCode() != GLFW_KEY_SPACE)
                return;

            if (_phase == CombatPhase::PlayerTurn)
                endPlayerTurn();
            else if (_phase == CombatPhase::ResolveTurn)
                startNewTurn();
        });

    startNewTurn();
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
    _playerPatternComponent->clearPlacedPatterns();
    _playerPatternComponent->clearPreview();
    _playerPatternComponent->clearActivePattern();

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
    _game->getEngine()->getInput()->OnKeyPressed.removeListener(_endTurnListenerId);
}

void game::CombatState::onUpdate(float dt) {

}

void game::CombatState::startNewTurn() {
    _phase = CombatPhase::EnemyPlanning;

    //for (auto* cell : _currentEnemy->getTerritory()) {
    //    cell->setDirty(true);
    //}

    generateEnemyBlockedCells();

    _phase = CombatPhase::PlayerTurn;
}

void game::CombatState::endPlayerTurn() {

    _phase = CombatPhase::ResolveTurn;

    //_player->clearTerritory();
    _playerPatternComponent->clearPlacedPatterns();
    _playerPatternComponent->clearPreview();
    _playerPatternComponent->clearActivePattern();

    //resolvePlayerPatterns();

    //resolveEnemyAction();
}

void game::CombatState::generateEnemyBlockedCells() {

    for (auto* cell : _currentEnemy->getBlockedCells()) {
        cell->setDirty(true);
    }

    _currentEnemy->clearBlockedCells();

    if (!_currentEnemy)
        return;

    std::vector<HexCell*> territory;

    for (auto* cell : _currentEnemy->getTerritory()) {

        if (!cell)
            continue;

        territory.push_back(cell);
    }

    if (territory.empty())
        return;

    std::shuffle(territory.begin(), territory.end(), std::mt19937(std::random_device{}()));

    size_t count = std::max<size_t>(1, territory.size() / 3);

    for (size_t i = 0; i < count; ++i) {
        auto* cell = territory[i];

        _currentEnemy->addBlockedCell(cell);

        auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");
        auto* world = worldGO->getComponent<game::World>();

        auto* transform = world->getHexTransformByCell(*cell);
        if (!transform)
            continue;

        auto* mesh = transform->getOwner()->getComponent<dzemikk::MeshRenderer>();
        if (!mesh)
            continue;

        mesh->setColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
    }
}