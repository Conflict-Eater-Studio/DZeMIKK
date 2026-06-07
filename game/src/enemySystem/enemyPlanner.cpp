#include "enemySystem/enemyPlanner.h"
#include "enemySystem/enemyEntity.h"
#include "enemySystem/enemyPatternComponent.h"
#include "map/HexPattern.h"
#include "game.h"

#include <ecs/scene.h>
#include <ecs/gameobject.h>

std::vector<game::PlannedPattern> game::EnemyPlanner::planTurn(EnemyEntity* enemy,
                                                         EnemyPatternComponent* patternComponent,
                                                         HexGrid* grid, float coverage) {
    return fillEnemyBoard(enemy, patternComponent, grid, coverage);
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

float game::EnemyPlanner::scorePattern(const EnemyEntity* enemy, const HexPattern& pattern) {
    float typeWeight = getTypeWeight(enemy, pattern.getType());

    return typeWeight * pattern.getEffectStrength();
}

game::HexPattern::Type game::EnemyPlanner::choosePatternType(const EnemyEntity* enemy) {
    const auto& weights = enemy->getActionWeights();

    float attackWeight = std::max(0.0F, weights.attack);
    float defenseWeight = std::max(0.0F, weights.defense);
    float healWeight = std::max(0.0F, weights.heal);

    float totalWeight = attackWeight + defenseWeight + healWeight;

    if (totalWeight <= 0.0F) {
        return HexPattern::Type::ATK;
    }

    static std::mt19937 rng(std::random_device{}());

    std::uniform_real_distribution<float> dist(0.0F, totalWeight);

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
                                       HexGrid* grid, const std::vector<HexCell*>& availableCells) {
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

                candidate.score = scorePattern(enemy, pattern);

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

    size_t topCount = std::min<size_t>(5, candidates.size());

    static std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution<size_t> dist(0, topCount - 1);

    return candidates[dist(rng)];
}

std::vector<game::PlannedPattern>
game::EnemyPlanner::fillEnemyBoard(EnemyEntity* enemy, EnemyPatternComponent* patternComponent,
                                   HexGrid* grid, float coverage) {

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

        auto candidates = generateCandidates(enemy, patternComponent, grid, availableCells);

        HexPattern::Type desiredType = choosePatternType(enemy);

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