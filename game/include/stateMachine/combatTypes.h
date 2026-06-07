#pragma once

#include "map/HexCell.h"
#include "map/HexPattern.h"

#include <vector>

namespace game {

/**
 * @brief Defines the current phase of the combat flow.
 */
enum class CombatPhase : std::uint8_t { PreparingBoard, EnemyPlanning, PlayerTurn, ResolveTurn };

/**
 * @brief Represents a potential pattern placement evaluated by the AI.
 *
 * Stores the pattern, the cells it would occupy, and the calculated score
 * used to compare placement candidates.
 */
struct PlacementCandidate {
    HexPattern* pattern = nullptr;
    std::vector<HexCell*> cells;
    float score = 0.0F;
};

/**
 * @brief Represents a pattern selected for execution during combat.
 *
 * Contains the pattern type, its calculated strength, and the cells affected
 * by the pattern.
 */
struct PlannedPattern {
    HexPattern::Type type;
    float strength = 0.0F;
    std::vector<HexCell*> cells;
};

} // namespace game