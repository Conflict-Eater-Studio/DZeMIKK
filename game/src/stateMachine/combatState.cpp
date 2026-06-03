#include "stateMachine/combatState.h"
#include "enemySystem/enemyEntity.h"
#include "map/HexGrid.h"
#include "map/HexCell.h"
#include "enemySystem/enemyManager.h"
#include "player/playerMovement.h"
#include "enemySystem/combatArenaBuilder.h"
#include "player/playerPatternComponent.h"

#include "game.h"
#include "camera/cameraController.h"
#include <ecs/scene.h>
#include <ecs/gameobject.h>
#include <ecs/components/meshRenderer.h>
#include <assetManager/assetHandle.h>
#include <iostream>
#include <enemySystem/enemyPatternComponent.h>
#include "ui/combatUIPanel.h"
#include "healthSystem.h"

#include "gameStateMachine.h"
#include "stateMachine/explorationState.h"

const char* patternTypeToString(game::HexPattern::Type type) {
    switch (type) {
    case game::HexPattern::Type::ATK:
        return "ATK";

    case game::HexPattern::Type::DEF:
        return "DEF";

    case game::HexPattern::Type::HEAL:
        return "HEAL";

    default:
        return "UNKNOWN";
    }
}

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

    auto enemyHealthbarGO =
        _game->getCurrentScene().get()->findGameObjectByName("Enemy_Healthbar_Slider");
    auto* enemyHealthbarSystem = enemyHealthbarGO->getComponent<HealthSystem>();
    enemyHealthbarSystem->setMaxHealth(_currentEnemy->getHp(), true);

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
    if (_shouldLeaveCombat) {
        _shouldLeaveCombat = false;
        _game->setExplorationState();
        return;
    }
}

void game::CombatState::startNewTurn() {
    _phase = CombatPhase::EnemyPlanning;
    
    _playerPatternComponent->clearPlacedPatterns();
    _playerPatternComponent->clearPreview();
    _playerPatternComponent->clearActivePattern();

    for (auto* cell : _currentEnemy->getTerritory()) {
        cell->setDirty(true);
    }

    auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");
    auto* world = worldGO->getComponent<World>();

    for (auto* cell : _currentEnemy->getBlockedCells()) {
        auto* transform = world->getHexTransformByCell(*cell);
        auto* mesh = transform->getOwner()->getComponent<dzemikk::MeshRenderer>();
        mesh->setColor(glm::vec4(0.0F, 0.0F, 0.5F, 1.0F));
    }

    auto* enemyManagerGO = _game->getCurrentScene().get()->findGameObjectByName("EnemyManager");
    auto* patternComponent = enemyManagerGO->getComponent<EnemyPatternComponent>();
    patternComponent->clearUsage();
    
    generateEnemyBlockedCells();
    auto enemyPanel = _game->getCurrentScene().get()->findGameObjectByName("Enemy_Panel");
    auto* enemyPanelUI = enemyPanel->getComponent<CombatUIPanel>();
    enemyPanelUI->refreshVisuals();

    _phase = CombatPhase::PlayerTurn;
}

void game::CombatState::endPlayerTurn() {

    _phase = CombatPhase::ResolveTurn;

    resolveConflict();
    showEnemyPlannedPatterns();
}

void game::CombatState::generateEnemyBlockedCells() {
    _plannedPatterns.clear();
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

    fillEnemyBoard(0.75f);
}

float game::CombatState::getTypeWeight(const EnemyEntity* enemy, HexPattern::Type type) {
    const auto& weights = enemy->getActionWeights();

    switch (type) {
    case HexPattern::Type::ATK:
        return weights.attack;

    case HexPattern::Type::DEF:
        return weights.defense;

    case HexPattern::Type::HEAL:
        return weights.heal;

    default:
        return 0.0f;
    }
}

float game::CombatState::scorePattern(const EnemyEntity* enemy, const HexPattern& pattern) {
    float typeWeight = getTypeWeight(enemy, pattern.getType());

    return typeWeight * pattern.getEffectStrength();
}

std::optional<game::CombatState::PlacementCandidate>
game::CombatState::chooseCandidate(std::vector<PlacementCandidate>& candidates) {
    if (candidates.empty())
        return std::nullopt;

    std::sort(
        candidates.begin(), candidates.end(),
        [](const PlacementCandidate& a, const PlacementCandidate& b) { return a.score > b.score; });

    size_t topCount = std::min<size_t>(5, candidates.size());

    static std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution<size_t> dist(0, topCount - 1);

    return candidates[dist(rng)];
}

std::vector<game::CombatState::PlacementCandidate>
game::CombatState::generateCandidates(const std::vector<HexCell*>& availableCells) {
    std::vector<PlacementCandidate> result;

    auto* enemyManagerGO = _game->getCurrentScene().get()->findGameObjectByName("EnemyManager");
    auto* patternComponent = enemyManagerGO->getComponent<EnemyPatternComponent>();

    if (!patternComponent)
        return result;

    for (const auto& entry : patternComponent->getPatterns()) {

        const auto& pattern = entry.pattern;

        for (auto* anchor : availableCells) {

            if (!anchor)
                continue;

            HexPattern rotatedPattern = pattern;

            for (int rotation = 0; rotation < 6; ++rotation) {

                if (rotation > 0) {
                    rotatedPattern.rotate(HexPattern::Rotation::Clockwise);
                }

                std::vector<HexCell*> cells;

                if (!tryPlacePattern(anchor, rotatedPattern, cells))
                    continue;

                PlacementCandidate candidate;

                candidate.pattern = const_cast<HexPattern*>(&pattern);

                candidate.cells = std::move(cells);

                candidate.score = scorePattern(_currentEnemy, pattern);

                result.push_back(std::move(candidate));
            }
        }
    }

    return result;
}

void game::CombatState::fillEnemyBoard(float coverage) {
    if (!_currentEnemy)
        return;

    std::vector<HexCell*> availableCells;

    for (auto* cell : _currentEnemy->getTerritory()) {

        if (!cell)
            continue;

        availableCells.push_back(cell);
    }

    if (availableCells.empty())
        return;

    const size_t targetFill =
        std::max<size_t>(1, static_cast<size_t>(availableCells.size() * coverage));

    size_t occupiedCount = 0;

    while (occupiedCount < targetFill) {

        auto candidates = generateCandidates(availableCells);

        HexPattern::Type desiredType = choosePatternType();

        std::vector<PlacementCandidate> filteredCandidates;

        for (auto& candidate : candidates) {

            if (!candidate.pattern)
                continue;

            if (candidate.pattern->getType() == desiredType) {
                filteredCandidates.push_back(candidate);
            }
        }

        if (filteredCandidates.empty()) {
            filteredCandidates = std::move(candidates);
        }

        auto chosen = chooseCandidate(filteredCandidates);

        if (!chosen.has_value())
            break;

        auto* enemyManagerGO = _game->getCurrentScene().get()->findGameObjectByName("EnemyManager");

        auto* patternComponent = enemyManagerGO->getComponent<EnemyPatternComponent>();

        patternComponent->registerPatternUsage(chosen->pattern);

        PlannedPattern planned;
        planned.type = chosen->pattern->getType();
        planned.strength = chosen->pattern->getEffectStrength();
        planned.cells = chosen->cells;

        _plannedPatterns.push_back(std::move(planned));

        for (auto* cell : chosen->cells) {
            if (!cell)
                continue;

            if (_currentEnemy->isCellBlocked(cell))
                continue;

            const auto& coord = cell->getCoord();

            _currentEnemy->addBlockedCell(cell);

            occupiedCount++;

            if (occupiedCount >= targetFill)
                break;
        }

        availableCells.erase(
            std::remove_if(availableCells.begin(), availableCells.end(),
                           [this](HexCell* cell) { return _currentEnemy->isCellBlocked(cell); }),
            availableCells.end());

        if (availableCells.empty())
            break;
    }
}

bool game::CombatState::tryPlacePattern(HexCell* anchor, const HexPattern& pattern,
                                        std::vector<HexCell*>& outCells) {
    outCells.clear();

    if (!anchor || !_currentEnemy)
        return false;

    auto* grid = _game->getHexGrid();

    if (!grid)
        return false;

    const HexCoord anchorCoord = anchor->getCoord();

    for (const auto& offset : pattern.getHexes()) {

        HexCoord targetCoord = anchorCoord + offset;

        auto targetCellPtr = grid->getCell(targetCoord);

        if (!targetCellPtr)
            return false;

        HexCell* targetCell = targetCellPtr.get();

        if (!_currentEnemy->getTerritory().contains(targetCell))
            return false;

        if (_currentEnemy->isCellBlocked(targetCell))
            return false;

        outCells.push_back(targetCell);
    }

    return true;
}

glm::vec4 game::CombatState::getPatternColor(HexPattern::Type type) {
    switch (type) {
    case HexPattern::Type::ATK:
        return {1.f, 0.0f, 0.0f, 1.0f};

    case HexPattern::Type::DEF:
        return {0.0f, 0.0f, 1.f, 1.0f};

    case HexPattern::Type::HEAL:
        return {0.0f, 1.f, 0.0f, 1.0f};

    default:
        return {0.3f, 0.3f, 0.3f, 1.0f};
    }
}

void game::CombatState::showEnemyPlannedPatterns() {

    auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");

    auto* world = worldGO->getComponent<World>();

    for (const auto& pattern : _plannedPatterns) {

        glm::vec4 color = getPatternColor(pattern.type);

        for (auto* cell : pattern.cells) {

            if (!cell)
                continue;

            const auto& coord = cell->getCoord();

            auto* transform = world->getHexTransformByCell(*cell);

            if (!transform)
                continue;

            auto* mesh = transform->getOwner()->getComponent<dzemikk::MeshRenderer>();

            if (!mesh)
                continue;

            mesh->setColor(color);
        }
    }
}

game::HexPattern::Type game::CombatState::choosePatternType() const {
    const auto& weights = _currentEnemy->getActionWeights();

    float attackWeight = std::max(0.0f, weights.attack);
    float defenseWeight = std::max(0.0f, weights.defense);
    float healWeight = std::max(0.0f, weights.heal);

    float totalWeight = attackWeight + defenseWeight + healWeight;

    if (totalWeight <= 0.0f)
        return HexPattern::Type::ATK;

    static std::mt19937 rng(std::random_device{}());

    std::uniform_real_distribution<float> dist(0.0f, totalWeight);

    float roll = dist(rng);

    if (roll < attackWeight)
        return HexPattern::Type::ATK;

    roll -= attackWeight;

    if (roll < defenseWeight)
        return HexPattern::Type::DEF;

    return HexPattern::Type::HEAL;
}

void game::CombatState::resolveConflict() {
    struct CellEffect {
        float atk = 0.0f;
        float def = 0.0f;
        float heal = 0.0f;
    };

    std::unordered_map<HexCoord, CellEffect> playerEffects;
    std::unordered_map<HexCoord, CellEffect> enemyEffects;

    for (const auto& placed : _playerPatternComponent->getPlacedPatterns()) {

        const auto& pattern = placed.pattern;

        for (const auto& offset : pattern.getHexes()) {

            auto& effect = playerEffects[offset];

            switch (pattern.getType()) {
            case HexPattern::Type::ATK:
                effect.atk += pattern.getEffectStrength();
                break;

            case HexPattern::Type::DEF:
                effect.def += pattern.getEffectStrength();
                break;

            case HexPattern::Type::HEAL:
                effect.heal += pattern.getEffectStrength();
                break;

            default:
                break;
            }
        }
    }

    for (const auto& pattern : _plannedPatterns) {

        for (auto* cell : pattern.cells) {

            if (!cell)
                continue;

            HexCoord offset = _currentEnemy->getCell().get()->getCoord() -  cell->getCoord();

            auto& effect = enemyEffects[offset];

            switch (pattern.type) {
            case HexPattern::Type::ATK:
                effect.atk += pattern.strength;
                break;

            case HexPattern::Type::DEF:
                effect.def += pattern.strength;
                break;

            case HexPattern::Type::HEAL:
                effect.heal += pattern.strength;
                break;

            default:
                break;
            }
        }
    }

    float damageToPlayer = 0.0f;
    float damageToEnemy = 0.0f;

    float healPlayer = 0.0f;
    float healEnemy = 0.0f;

    std::unordered_set<HexCoord> allOffsets;

    for (const auto& [coord, _] : playerEffects)
        allOffsets.insert(coord);

    for (const auto& [coord, _] : enemyEffects)
        allOffsets.insert(coord);

    for (const auto& coord : allOffsets) {

        CellEffect player;
        CellEffect enemy;

        if (auto it = playerEffects.find(coord); it != playerEffects.end())
            player = it->second;

        if (auto it = enemyEffects.find(coord); it != enemyEffects.end())
            enemy = it->second;

        float dmgToEnemy = std::max(0.0f, player.atk - enemy.def);
        float dmgToPlayer = std::max(0.0f, enemy.atk - player.def);
    }

    for (const auto& coord : allOffsets) {

        CellEffect player;
        CellEffect enemy;

        if (auto it = playerEffects.find(coord); it != playerEffects.end())
            player = it->second;

        if (auto it = enemyEffects.find(coord); it != enemyEffects.end())
            enemy = it->second;

        damageToEnemy += std::max(0.0f, player.atk - enemy.def);

        damageToPlayer += std::max(0.0f, enemy.atk - player.def);

        healPlayer += player.heal;
        healEnemy += enemy.heal;
    }


    auto* playerHealth = _game->getCurrentScene().get()
                             ->findGameObjectByName("Player_Healthbar_Slider")
                             ->getComponent<HealthSystem>();

    if (playerHealth) {
        playerHealth->damage(static_cast<int>(std::round(damageToPlayer)));
        playerHealth->heal(static_cast<int>(std::round(healPlayer)));
    }

    auto* enemyHealth = _game->getCurrentScene().get()
                            ->findGameObjectByName("Enemy_Healthbar_Slider")
                            ->getComponent<HealthSystem>();

    if (enemyHealth) {
        enemyHealth->damage(static_cast<int>(std::round(damageToEnemy)));
        enemyHealth->heal(static_cast<int>(std::round(healEnemy)));
    }

    if (playerHealth->isDead() || enemyHealth->isDead()) {
        _shouldLeaveCombat = true;
    }
}