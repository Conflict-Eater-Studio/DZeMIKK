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
#include "map/HexChunk.h"
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
#include <ecs/components/transform.h>
#include <enemySystem/enemyPatternComponent.h>
#include <iostream>

void game::CombatState::onEnter() {
    _phase = CombatPhase::PreparingBoard;

    _game->getCameraController()->setMode(CameraController::Mode::Combat);
    _game->enableCombatUI(true);

    initializeCombat();

    if (!_arenaCenterCell) {
        return;
    }

    setupInput();
    setupEnemyHealth();

    collectAnimatedHexes();

    _boardTransition = 0.0F;
    _enterAnimation = true;

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
    if (_exitAnimation) {

        _boardTransition -= dt * _animationExitSpeed;

        if (_boardTransition <= 0.0F) {

            _boardTransition = 0.0F;
            _exitAnimation = false;

            _game->setExplorationState();
            return;
        }

        updateBoardVisibility(_boardTransition, true);
    }

    if (_enterAnimation) {
        _boardTransition += dt * _animationEnterSpeed;

        if (_boardTransition >= 1.0F) {
            _boardTransition = 1.0F;
            _enterAnimation = false;
        }

        updateBoardVisibility(_boardTransition, false);
    }
}

void game::CombatState::startNewTurn() {
    _roundCount++;
    auto* textGO = _game->getCurrentScene()
                       .get()
                       ->findGameObjectByName("Round_Number")
                       ->findChildByName("Text");
    auto* textRenderer = textGO->getComponent<dzemikk::UITextRenderer>();
    std::string textFill = "ROUND " + std::to_string(_roundCount);
    textRenderer->text = textFill;

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
        _exitAnimation = true;
        _boardTransition = 1.0F;
    }
}

void game::CombatState::updateBoardVisibility(float factor, bool exiting) {
    constexpr float DelayPerRing = 0.05F;

    for (auto& hex : _hiddenHexes) {

        float delay = (float)hex.distance * DelayPerRing;

        float localFactor = 0.0F;

        if (!exiting) {
            localFactor = (factor - delay) / (1.0F - delay);

        } else {

            localFactor = ((1.0F - factor) - delay) / (1.0F - delay);
            localFactor = 1.0F - localFactor;
        }

        localFactor = glm::clamp(localFactor, 0.0F, 1.0F);

        auto pos = hex.transform->getPosition();

        pos.y = glm::mix(hex.startY, hex.startY + _hiddenOffsetY, localFactor);

        hex.transform->setPosition(pos);
    }
}

void game::CombatState::initializeCombat() {
    auto* scene = _game->getCurrentScene().get();

    auto* playerGO = scene->findGameObjectByName("Player");
    _player = playerGO->getComponent<PlayerEntity>();

    auto* worldGO = scene->findGameObjectByName("World");
    auto* world = worldGO->getComponent<World>();

    auto* enemyManagerGO = scene->findGameObjectByName("EnemyManager");
    auto* enemyManager = enemyManagerGO->getComponent<EnemyManager>();

    auto playerCell = _game->getHexGrid()->findCellByEntity(_player);

    _currentEnemy = enemyManager->getEnemyByCell(playerCell.get());

    auto arena = CombatArenaBuilder::build(_currentEnemy, _player, _game->getHexGrid(), world);

    _arenaCenterCell = arena.centerCell.get();

    if (!_arenaCenterCell) {
        return;
    }

    if (auto* movement = _player->getOwner()->getComponent<PlayerMovement>()) {

        movement->stopMovement();
    }

    _player->teleportTo(arena.centerCell);

    _playerPatternComponent = playerGO->getComponent<PlayerPatternComponent>();
}

void game::CombatState::setupInput() {
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
}

void game::CombatState::setupEnemyHealth() {
    auto* enemyHealthGO = _game->getCurrentScene().get()
                              ->findGameObjectByName("Enemy_Avatar_Panel")
                              ->findDescendantByName("Health_Holder");

    auto* enemyHealthSystem = enemyHealthGO->getComponent<HealthSystem>();

    enemyHealthSystem->setMaxHealth(static_cast<float>(_currentEnemy->getHp()), true);
}

void game::CombatState::collectAnimatedHexes() {

    _hiddenHexes.clear();

    auto* world = _game->getCurrentScene().get()->findGameObjectByName("World")->getComponent<World>();

    const HexCoord centerCoord = _arenaCenterCell->getCoord();

    int visibleRadius = 18;

    auto playerCellPtr = _player->getCell();
    auto enemyCellPtr = _currentEnemy->getCell();

    for (const auto& chunk : _game->getHexGrid()->getChunks()) {

        for (const auto& [coord, cellPtr] : chunk.second->getHexes()) {

            auto* cell = cellPtr.get();

            if (!shouldAnimateCell(cell, centerCoord, visibleRadius, playerCellPtr.get(),
                                   enemyCellPtr.get())) {
                continue;
            }

            addCellToAnimation(cell, world, centerCoord);
        }
    }
}

bool game::CombatState::shouldAnimateCell(HexCell* cell, const HexCoord& centerCoord,
                                          int visibleRadius, HexCell* playerCell,
                                          HexCell* enemyCell) const {
    if (!cell) {
        return false;
    }

    if (HexCoord::distance(cell->getCoord(), centerCoord) > visibleRadius) {
        return false;
    }

    if (_player->getTerritory().contains(cell)) {
        return false;
    }

    if (_currentEnemy->getTerritory().contains(cell)) {
        return false;
    }

    if (cell == playerCell || cell == enemyCell) {
        return false;
    }

    return true;
}

void game::CombatState::addCellToAnimation(HexCell* cell, World* world,
                                           const HexCoord& centerCoord) {
    int distance = HexCoord::distance(cell->getCoord(), centerCoord);

    if (auto* transform = world->getHexTransformByCell(*cell)) {

        _hiddenHexes.push_back({transform, transform->getPosition().y, distance});
    }

    auto* entity = cell->getEntity();

    if (!entity || !entity->getOwner()) {
        return;
    }

    auto* entityTransform = entity->getOwner()->getComponent<dzemikk::Transform>();

    if (!entityTransform) {
        return;
    }

    _hiddenHexes.push_back({entityTransform, entityTransform->getPosition().y, distance});
}

