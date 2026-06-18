#ifndef GAME_ENEMY_PLANNER_H
#define GAME_ENEMY_PLANNER_H

#include "stateMachine/combatTypes.h"

#include <optional>
#include <vector>
#include <player/playerPatternComponent.h>
#include "enemySystem/behaviorTree/bTNode.h"

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
     * @brief Constructs a new EnemyPlanner instance.
     */
    EnemyPlanner();

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
    std::vector<PlannedPattern> planTurn(Game* game, EnemyEntity* enemy,
                                         EnemyPatternComponent* patternComponent, HexGrid* grid,
                                         float coverage,
                                         const PlayerPatternStatsComponent* playerStats);

  private:
    /**
     * @brief Root node of the enemy behavior tree.
     *
     * Used to evaluate combat context and produce modifiers that influence
     * enemy decision making.
     */
    std::unique_ptr<BTNode> _root;

    /**
     * @brief Returns the utility weight for a pattern type.
     *
     * Combines enemy characteristics and behavior tree modifiers to determine
     * how desirable a pattern type is in the current combat context.
     *
     * @param enemy Enemy entity being evaluated.
     * @param modifiers Behavior tree context modifiers.
     * @param type Pattern type being evaluated.
     *
     * @return float Utility weight of the pattern type.
     */
    static float getUtilityWeight(const EnemyEntity* enemy,
                                         const BTNode::ContextModifiers& modifiers, HexPattern::Type type);

    /**
     * @brief Calculates the score of a pattern for the given enemy.
     *
     * @param enemy Enemy entity being evaluated.
     * @param pattern Pattern to score.
     *
     * @return float Calculated pattern score.
     */
    static float scorePattern(const EnemyEntity* enemy, const BTNode::ContextModifiers& modifiers,
                                            const HexPattern& pattern);

    /**
     * @brief Chooses a pattern type for the enemy.
     *
     * @param enemy Enemy entity being evaluated.
     *
     * @return HexPattern::Type Selected pattern type.
     */
    static HexPattern::Type choosePatternType(const EnemyEntity* enemy,
                                              const BTNode::ContextModifiers& modifiers);

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
                       const std::vector<HexCell*>& availableCells,
                       const BTNode::ContextModifiers& modifiers);

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
                                               HexGrid* grid, float coverage,
                                               const BTNode::ContextModifiers& modifiers);

    /**
     * @brief Evaluates the enemy behavior tree.
     *
     * Analyzes the current combat situation and produces context modifiers
     * that influence pattern selection and placement decisions.
     *
     * @param game Active game instance.
     * @param enemy Enemy entity being evaluated.
     * @param playerStats Player pattern statistics used for decision making.
     *
     * @return BTNode::ContextModifiers Computed behavior modifiers.
     */
    BTNode::ContextModifiers evaluateBehaviorTree(Game* game, EnemyEntity* enemy,
                                       const PlayerPatternStatsComponent* playerStats);

    static size_t getMaxPatternSize(const EnemyEntity* enemy, const HexGrid* grid);
};

} // namespace game

#endif