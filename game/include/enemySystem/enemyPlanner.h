#ifndef GAME_ENEMY_PLANNER_H
#define GAME_ENEMY_PLANNER_H

#include "stateMachine/combatTypes.h"

#include <optional>
#include <vector>

class Game;

namespace game {

class EnemyEntity;
class EnemyPatternComponent;
class HexGrid;
class HexCell;
class HexPattern;

/**
 * @brief Generates combat plans for enemy entities.
 *
 * Selects patterns, evaluates possible placements, and produces a set of
 * planned pattern executions for the enemy turn.
 */
class EnemyPlanner {
  public:
    /**
     * @brief Creates a combat plan for the specified enemy.
     *
     * Generates and places patterns on the grid according to the enemy's
     * available patterns and desired board coverage.
     *
     * @param enemy Enemy entity being planned.
     * @param patternComponent Component containing enemy patterns.
     * @param grid Combat grid used for placement.
     * @param coverage Desired board coverage ratio.
     *
     * @return std::vector<PlannedPattern> Planned enemy pattern executions.
     */
    std::vector<PlannedPattern> planTurn(EnemyEntity* enemy,
                                         EnemyPatternComponent* patternComponent, HexGrid* grid,
                                         float coverage = 0.75F);

  private:
    /**
     * @brief Returns the selection weight for a pattern type.
     *
     * @param enemy Enemy entity being evaluated.
     * @param type Pattern type.
     *
     * @return float Weight assigned to the pattern type.
     */
    static float getTypeWeight(const EnemyEntity* enemy, HexPattern::Type type);

    /**
     * @brief Calculates the score of a pattern for the given enemy.
     *
     * @param enemy Enemy entity being evaluated.
     * @param pattern Pattern to score.
     *
     * @return float Calculated pattern score.
     */
    static float scorePattern(const EnemyEntity* enemy, const HexPattern& pattern);

    /**
     * @brief Chooses a pattern type for the enemy.
     *
     * @param enemy Enemy entity being evaluated.
     *
     * @return HexPattern::Type Selected pattern type.
     */
    static HexPattern::Type choosePatternType(const EnemyEntity* enemy);

    /**
     * @brief Generates all valid placement candidates for the enemy.
     *
     * @param enemy Enemy entity being planned.
     * @param patternComponent Component containing enemy patterns.
     * @param grid Combat grid used for placement.
     * @param availableCells Cells available for placement.
     *
     * @return std::vector<PlacementCandidate> Generated placement candidates.
     */
    static std::vector<PlacementCandidate>
    generateCandidates(EnemyEntity* enemy, EnemyPatternComponent* patternComponent, HexGrid* grid,
                       const std::vector<HexCell*>& availableCells);

    /**
     * @brief Attempts to place a pattern on the grid.
     *
     * @param enemy Enemy entity being planned.
     * @param grid Combat grid used for placement.
     * @param anchor Anchor cell used as the placement origin.
     * @param pattern Pattern being placed.
     * @param outCells Receives the cells occupied by the pattern.
     *
     * @return true If placement succeeded.
     * @return false If placement failed.
     */
    static bool tryPlacePattern(EnemyEntity* enemy, HexGrid* grid, HexCell* anchor,
                                const HexPattern& pattern, std::vector<HexCell*>& outCells);

    /**
     * @brief Selects a placement candidate from the available options.
     *
     * @param candidates Available placement candidates.
     *
     * @return std::optional<PlacementCandidate> Selected candidate if available.
     */
    static std::optional<PlacementCandidate>
    chooseCandidate(std::vector<PlacementCandidate>& candidates);

    /**
     * @brief Fills the enemy board with planned pattern placements.
     *
     * @param enemy Enemy entity being planned.
     * @param patternComponent Component containing enemy patterns.
     * @param grid Combat grid used for placement.
     * @param coverage Desired board coverage ratio.
     *
     * @return std::vector<PlannedPattern> Planned pattern executions.
     */
    std::vector<PlannedPattern> fillEnemyBoard(EnemyEntity* enemy,
                                               EnemyPatternComponent* patternComponent,
                                               HexGrid* grid, float coverage);
};

} // namespace game

#endif