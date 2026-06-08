#include "enemySystem/enemyPlanner.h"
#include "enemySystem/enemyEntity.h"
#include "enemySystem/enemyPatternComponent.h"
#include "map/HexPattern.h"
#include "game.h"

#include <ecs/scene.h>
#include <ecs/gameobject.h>
#include <healthSystem.h>

#include "player/playerPatternStatsComponent.h"

std::vector<game::PlannedPattern>
game::EnemyPlanner::planTurn(Game* game, EnemyEntity* enemy,
                             EnemyPatternComponent* patternComponent,
                             HexGrid* grid, float coverage,
                             const PlayerPatternStatsComponent* playerStats) {
    ContextModifiers modifiers = evaluateBehaviorTree(game, enemy, playerStats);

    return fillEnemyBoard(enemy, patternComponent, grid, coverage, modifiers);
}

float game::EnemyPlanner::getTypeWeight(const EnemyEntity* enemy, HexPattern::Type type) {
    const auto& weights = enemy->getActionWeights();

    switch (type) {
    case HexPattern::Type::ATK:
        return weights.attack;

    case HexPattern::Type::DEF:
        return weights.defense;

    case HexPattern::Type::HEAL:
        return weights.heal;

    default:
        return 0.0F;
    }
}

float game::EnemyPlanner::getUtilityWeight(const EnemyEntity* enemy, const ContextModifiers& modifiers,
                                     HexPattern::Type type) {
    const auto& weights = enemy->getActionWeights();

    switch (type) {

    case HexPattern::Type::ATK:
        return weights.attack * modifiers.attack;

    case HexPattern::Type::DEF:
        return weights.defense * modifiers.defense;

    case HexPattern::Type::HEAL:
        return weights.heal * modifiers.heal;

    default:
        return 0.0f;
    }
}

float game::EnemyPlanner::scorePattern(const EnemyEntity* enemy, const ContextModifiers& modifiers,
                                 const HexPattern& pattern) {
    float utility = getUtilityWeight(enemy, modifiers, pattern.getType());

    return utility * pattern.getEffectStrength();
}

game::HexPattern::Type game::EnemyPlanner::choosePatternType(const EnemyEntity* enemy,
                                                             const ContextModifiers& modifiers) {

    float attackWeight = std::max(0.0f, getUtilityWeight(enemy, modifiers, HexPattern::Type::ATK));

    float defenseWeight = std::max(0.0f, getUtilityWeight(enemy, modifiers, HexPattern::Type::DEF));

    float healWeight = std::max(0.0f, getUtilityWeight(enemy, modifiers, HexPattern::Type::HEAL));

    float totalWeight = attackWeight + defenseWeight + healWeight;

    if (totalWeight <= 0.0f) {
        return HexPattern::Type::ATK;
    }

    static std::mt19937 rng(std::random_device{}());

    std::uniform_real_distribution<float> dist(0.0f, totalWeight);

    float roll = dist(rng);

    if (roll < attackWeight) {
        return HexPattern::Type::ATK;
    }

    roll -= attackWeight;

    if (roll < defenseWeight) {
        return HexPattern::Type::DEF;
    }

    return HexPattern::Type::HEAL;
}

std::vector<game::PlacementCandidate>
game::EnemyPlanner::generateCandidates(EnemyEntity* enemy, EnemyPatternComponent* patternComponent,
                                       HexGrid* grid, const std::vector<HexCell*>& availableCells,
                                       const ContextModifiers& modifiers) {
    std::vector<PlacementCandidate> result;

    if (!patternComponent) {
        return result;
    }

    for (const auto& entry : patternComponent->getPatterns()) {

        const auto& pattern = entry.pattern;

        for (auto* anchor : availableCells) {

            if (!anchor) {
                continue;
            }

            HexPattern rotatedPattern = pattern;

            for (int rotation = 0; rotation < 6; ++rotation) {

                if (rotation > 0) {
                    rotatedPattern.rotate(HexPattern::Rotation::Clockwise);
                }

                std::vector<HexCell*> cells;

                if (!tryPlacePattern(enemy, grid, anchor, rotatedPattern, cells)) {
                    continue;
                }

                PlacementCandidate candidate;

                candidate.pattern = const_cast<HexPattern*>(&pattern);

                candidate.cells = std::move(cells);

                candidate.score = scorePattern(enemy, modifiers, pattern);

                result.push_back(std::move(candidate));
            }
        }
    }

    return result;
}

bool game::EnemyPlanner::tryPlacePattern(EnemyEntity* enemy, HexGrid* grid, HexCell* anchor,
                                         const HexPattern& pattern,
                                         std::vector<HexCell*>& outCells) {
    outCells.clear();

    if (!anchor || !enemy || !grid) {
        return false;
    }

    const HexCoord anchorCoord = anchor->getCoord();

    for (const auto& offset : pattern.getHexes()) {

        HexCoord targetCoord = anchorCoord + offset;

        auto targetCellPtr = grid->getCell(targetCoord);

        if (!targetCellPtr) {
            return false;
        }

        HexCell* targetCell = targetCellPtr.get();

        if (!enemy->getTerritory().contains(targetCell)) {
            return false;
        }

        if (enemy->isCellBlocked(targetCell)) {
            return false;
        }

        outCells.push_back(targetCell);
    }

    return true;
}

std::optional<game::PlacementCandidate>
game::EnemyPlanner::chooseCandidate(std::vector<PlacementCandidate>& candidates) {
    if (candidates.empty()) {
        return std::nullopt;
    }

    std::sort(
        candidates.begin(), candidates.end(),
        [](const PlacementCandidate& a, const PlacementCandidate& b) { return a.score > b.score; });

    size_t topCount = std::min<size_t>(3, candidates.size());

    static std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution<size_t> dist(0, topCount - 1);

    return candidates[dist(rng)];
}

std::vector<game::PlannedPattern>
game::EnemyPlanner::fillEnemyBoard(EnemyEntity* enemy, EnemyPatternComponent* patternComponent,
                                   HexGrid* grid, float coverage,
                                   const ContextModifiers& modifiers) {

    std::vector<PlannedPattern> plannedPatterns;

    if (!enemy || !patternComponent || !grid) {
        return plannedPatterns;
    }

    enemy->clearBlockedCells();

    std::vector<HexCell*> availableCells;

    for (auto* cell : enemy->getTerritory()) {
        if (cell) {
            availableCells.push_back(cell);
        }
    }

    if (availableCells.empty()) {
        return plannedPatterns;
    }

    const size_t targetFill =
        std::max<size_t>(1, static_cast<size_t>(availableCells.size() * coverage));

    size_t occupiedCount = 0;

    while (occupiedCount < targetFill) {

        auto candidates = generateCandidates(enemy, patternComponent, grid, availableCells, modifiers);

        HexPattern::Type desiredType = choosePatternType(enemy, modifiers);

        std::vector<PlacementCandidate> filteredCandidates;

        for (auto& candidate : candidates) {

            if (!candidate.pattern) {
                continue;
            }

            if (candidate.pattern->getType() == desiredType) {
                filteredCandidates.push_back(candidate);
            }
        }

        if (filteredCandidates.empty()) {
            filteredCandidates = std::move(candidates);
        }

        auto chosen = chooseCandidate(filteredCandidates);

        if (!chosen.has_value()) {
            break;
        }

        patternComponent->registerPatternUsage(chosen->pattern);

        PlannedPattern planned;
        planned.type = chosen->pattern->getType();
        planned.strength = chosen->pattern->getEffectStrength();
        planned.cells = chosen->cells;

        plannedPatterns.push_back(std::move(planned));

        for (auto* cell : chosen->cells) {

            if (!cell) {
                continue;
            }

            if (enemy->isCellBlocked(cell)) {
                continue;
            }

            enemy->addBlockedCell(cell);

            ++occupiedCount;

            if (occupiedCount >= targetFill) {
                break;
            }
        }

        availableCells.erase(
            std::remove_if(availableCells.begin(), availableCells.end(),
                           [enemy](HexCell* cell) { return enemy->isCellBlocked(cell); }),
            availableCells.end());

        if (availableCells.empty()) {
            break;
        }
    }

    return plannedPatterns;
}

game::EnemyPlanner::ContextModifiers
game::EnemyPlanner::evaluateBehaviorTree(Game* game, game::EnemyEntity* enemy,
                                   const game::PlayerPatternStatsComponent* playerStats) {
    ContextModifiers modifiers;

    if (!enemy) {
        return modifiers;
    }

    //
    // ROOT
    // Selector
    //
    // ??? LowHealth
    // ??? PlayerAggressive
    // ??? PlayerDefensive
    // ??? Default
    //

    auto* enemyHealthGO = game->getCurrentScene()
                              .get()
                              ->findGameObjectByName("Enemy_Avatar_Panel")
                              ->findDescendantByName("Health_Holder");

    auto* enemyHealthSystem = enemyHealthGO->getComponent<HealthSystem>();
    float hpPercent = enemyHealthSystem->getCurrentHealth() / enemyHealthSystem->getMaxHealth();

    //
    // LowHealth
    //
    if (hpPercent < 0.40f) {

        modifiers.attack = 0.5f;
        modifiers.defense = 1.5f;
        modifiers.heal = 4.0f;

        return modifiers;
    }

    //
    // PlayerAggressive
    //
    if (playerStats) {

        float attackRatio = playerStats->getStats().getTypeRatio(HexPattern::Type::ATK);

        if (attackRatio > 0.50f) {

            modifiers.attack = 0.9f;
            modifiers.defense = 2.0f;
            modifiers.heal = 1.0f;

            return modifiers;
        }
    }

    //
    // PlayerDefensive
    //
    if (playerStats) {

        float defenseRatio = playerStats->getStats().getTypeRatio(HexPattern::Type::DEF);

        if (defenseRatio > 0.50f) {

            modifiers.attack = 2.2f;
            modifiers.defense = 0.8f;
            modifiers.heal = 1.0f;

            return modifiers;
        }
    }

    //
    // Default
    //
    return modifiers;
}