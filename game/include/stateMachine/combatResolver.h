#ifndef GAME_COMBAT_RESOLVER_H
#define GAME_COMBAT_RESOLVER_H

#include "map/HexCoord.h"
#include "stateMachine/combatTypes.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace game {

class PlayerPatternComponent;

/**
 * @brief Resolves combat interactions between player and enemy patterns.
 *
 * Calculates damage and healing effects produced by both sides based on
 * their planned patterns and affected hex cells.
 */
class CombatResolver {
  public:
    /**
     * @brief Contains the outcome of a combat resolution.
     */
    struct Result {
        float damageToPlayer = 0.0F;
        float damageToEnemy = 0.0F;

        float healToPlayer = 0.0F;
        float healToEnemy = 0.0F;
    };

    /**
     * @brief Resolves combat between player and enemy pattern executions.
     *
     * Computes the resulting damage and healing values applied to both
     * combatants based on their active patterns and positions.
     *
     * @param playerPatterns Player pattern component containing planned actions.
     * @param enemyPatterns Enemy planned patterns.
     * @param enemyCenter Center position of the enemy on the hex grid.
     *
     * @return Result Structure containing damage and healing totals.
     */
    [[nodiscard]] static Result resolve(const PlayerPatternComponent& playerPatterns,
                                        const std::vector<PlannedPattern>& enemyPatterns,
                                        const HexCoord& enemyCenter);

  private:
    /**
     * @brief Aggregated combat effects applied to a single cell.
     */
    struct CellEffect {
        float atk = 0.0F;
        float def = 0.0F;
        float heal = 0.0F;
    };
};

} // namespace game

#endif