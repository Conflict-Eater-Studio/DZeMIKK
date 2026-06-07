#include "stateMachine/combatState.h"

#include "camera/cameraController.h"
#include "enemySystem/combatArenaBuilder.h"
#include "enemySystem/enemyEntity.h"
#include "enemySystem/enemyManager.h"
#include "game.h"
#include "gameStateMachine.h"
#include "healthSystem.h"
#include "map/HexCell.h"
#include "map/HexGrid.h"
#include "player/playerMovement.h"
#include "player/playerPatternComponent.h"
#include "stateMachine/explorationState.h"
#include "ui/combatUIPanel.h"
#include "stateMachine/combatResolver.h"
#include "enemySystem/enemyPlanner.h"

#include <assetManager/assetHandle.h>
#include <ecs/components/meshRenderer.h>
#include <ecs/gameobject.h>
#include <ecs/scene.h>
#include <enemySystem/enemyPatternComponent.h>
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

    if (!arena.centerCell) {
        return;
    }

    auto* movement = _player->getOwner()->getComponent<PlayerMovement>();

    if (movement) {
        movement->stopMovement();
    }

    _player->teleportTo(arena.centerCell);

    _playerPatternComponent = playerGO->getComponent<PlayerPatternComponent>();

    _endTurnListenerId = _game->getEngine()->getInput()->OnKeyPressed.addListener(
        [this](dzemikk::KeyPressedEvent& e) {
            if (e.GetKeyCode() != GLFW_KEY_SPACE) {
                return;
            }

            if (_phase == CombatPhase::PlayerTurn) {
                endPlayerTurn();
            } else if (_phase == CombatPhase::ResolveTurn) {
                startNewTurn();
            }
        });

    auto* enemyHealthGO = _game->getCurrentScene()
                             .get()
                             ->findGameObjectByName("Enemy_Avatar_Panel")
                             ->findDescendantByName("Health_Holder");

    auto* enemyHealthSystem = enemyHealthGO->getComponent<game::HealthSystem>();

    enemyHealthSystem->setMaxHealth((float)_currentEnemy->getHp(), true);

    startNewTurn();
}

void game::CombatState::onExit() {

    _game->enableCombatUI(false);

    auto scene = _game->getCurrentScene();

    auto* grid = _game->getHexGrid();

    if (!grid || !_currentEnemy || !_player) {
        return;
    }

    for (auto* cell : _player->getTerritory()) {

        if (!cell) {
            continue;
        }

        cell->setType(HexCell::Type::Normal);
        cell->setDirty(true);
    }

    _player->clearTerritory();
    _playerPatternComponent->clearPlacedPatterns();
    _playerPatternComponent->deactivatePattern();

    const auto& enemyTerritory = _currentEnemy->getTerritory();

    for (auto* cell : enemyTerritory) {

        if (!cell) {
            continue;
        }

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
    if (_shouldLeaveCombat) {
        _shouldLeaveCombat = false;
        _game->setExplorationState();
        return;
    }
}

void game::CombatState::startNewTurn() {
    _phase = CombatPhase::EnemyPlanning;

    _playerPatternComponent->clearPlacedPatterns();
    _playerPatternComponent->deactivatePattern();

    for (auto* cell : _currentEnemy->getTerritory()) {
        cell->setDirty(true);
    }

    auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");
    auto* world = worldGO->getComponent<World>();

    auto* enemyManagerGO = _game->getCurrentScene().get()->findGameObjectByName("EnemyManager");
    auto* patternComponent = enemyManagerGO->getComponent<EnemyPatternComponent>();
    patternComponent->clearUsage();

    EnemyPlanner planner;

    _plannedPatterns.clear();
    _plannedPatterns =
        planner.planTurn(_currentEnemy, patternComponent, _game->getHexGrid(), 0.75F);

    auto* enemyPanel = _game->getCurrentScene().get()->findGameObjectByName("Enemy_Panel");
    auto* enemyPanelUI = enemyPanel->getComponent<CombatUIPanel>();
    enemyPanelUI->refreshVisuals();

    _phase = CombatPhase::PlayerTurn;
}

void game::CombatState::endPlayerTurn() {

    _phase = CombatPhase::ResolveTurn;

    resolveConflict();
    showEnemyPlannedPatterns();
}

glm::vec4 game::CombatState::getPatternColor(HexPattern::Type type) {
    switch (type) {
    case HexPattern::Type::ATK:
        return {1.0F, 0.0F, 0.0F, 1.0F};

    case HexPattern::Type::DEF:
        return {0.0F, 0.0F, 1.F, 1.0F};

    case HexPattern::Type::HEAL:
        return {0.0F, 1.F, 0.0F, 1.0F};

    default:
        return {0.3F, 0.3F, 0.3F, 1.0F};
    }
}

void game::CombatState::showEnemyPlannedPatterns() {

    auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");

    auto* world = worldGO->getComponent<World>();

    for (const auto& pattern : _plannedPatterns) {

        glm::vec4 color = getPatternColor(pattern.type);

        for (auto* cell : pattern.cells) {

            if (!cell) {
                continue;
            }

            const auto& coord = cell->getCoord();

            auto* transform = world->getHexTransformByCell(*cell);

            if (!transform) {
                continue;
            }

            auto* mesh = transform->getOwner()->getComponent<dzemikk::MeshRenderer>();

            if (!mesh) {
                continue;
            }

            mesh->setColor(color);
        }
    }
}

void game::CombatState::resolveConflict() {
    auto result = CombatResolver::resolve(*_playerPatternComponent, _plannedPatterns,
                                   _currentEnemy->getCell()->getCoord());

    auto* playerHealth = _game->getCurrentScene().get()->findGameObjectByName("Player_Avatar_Panel")
                             ->findDescendantByName("Health_Holder")
                             ->getComponent<HealthSystem>();

    if (playerHealth) {
        playerHealth->damage(result.damageToPlayer);
        playerHealth->heal(result.healToPlayer);
    }

    auto* enemyHealth = _game->getCurrentScene().get()
                            ->findGameObjectByName("Enemy_Avatar_Panel")
                            ->findDescendantByName("Health_Holder")
                            ->getComponent<HealthSystem>();

    if (enemyHealth) {
        enemyHealth->damage(result.damageToEnemy);
        enemyHealth->heal(result.healToEnemy);
    }

    if (enemyHealth->isDead()) {
        auto* grid = _game->getCurrentScene()
                         .get()
                         ->findGameObjectByTag("World")
                         ->getComponent<World>()
                         ->getGrid();

        auto enemyChunkId = grid->findChunkForCoord(_currentEnemy->getCell()->getCoord())->getId();
        auto enemyConfig = _currentEnemy->getConfig();
        for (const auto& childChunk : enemyConfig.blocksChunks) {
            grid->unlockBridge({enemyChunkId, childChunk}, _currentEnemy->getId());
        }
    }

    if (playerHealth->isDead() || enemyHealth->isDead()) {
        _shouldLeaveCombat = true;
    }
}
