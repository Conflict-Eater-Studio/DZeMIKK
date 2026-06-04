#ifndef GAME_ENEMY_ENTITY_H
#define GAME_ENEMY_ENTITY_H

#include "enemySystem/enemyActionWeights.h"
#include "enemySystem/enemyTypes.h"
#include "map/Entity.h"

namespace game {

/**
 * @brief Represents an enemy occupying a map cell.
 *
 * Stores enemy state, territory ownership, blocked cells,
 * combat attributes, personality traits and action weights.
 */
class EnemyEntity : public Entity {
  public:
    EnemyEntity() = default;

#pragma region Lifecycle

    /**
     * @brief Places the enemy on a map cell.
     *
     * @param cell Target cell.
     */
    void onEnter(HexCellPtr cell) override;

    /**
     * @brief Removes the enemy from its current cell.
     */
    void onExit() override;

#pragma endregion

#pragma region Territory

    /**
     * @brief Adds a cell to the enemy territory.
     *
     * @param cell Territory cell.
     */
    void addTerritoryCell(HexCell* cell);

    /**
     * @brief Returns all territory cells owned by the enemy.
     *
     * @return const auto& Territory cells.
     */
    [[nodiscard]] const std::unordered_set<HexCell*> getTerritory() const;

#pragma endregion

#pragma region Blocked cells

    /**
     * @brief Marks a cell as blocked for the enemy.
     *
     * @param cell Blocked cell.
     */
    void addBlockedCell(HexCell* cell);

    /**
     * @brief Returns all currently blocked cells.
     *
     * @return const auto& Blocked cells.
     */
    [[nodiscard]] const std::unordered_set<HexCell*> getBlockedCells() const;

    /**
     * @brief Clears all blocked cells.
     */
    void clearBlockedCells();

    /**
     * @brief Checks whether a cell is blocked.
     *
     * @param cell Cell to check.
     * @return true If the cell is blocked.
     * @return false Otherwise.
     */
    [[nodiscard]] bool isCellBlocked(HexCell* cell) const;

#pragma endregion

#pragma region Attributes

    void setHp(double hp);
    void setEnemyType(EnemyType type);
    void setEnemyPersonality(EnemyPersonality personality);

    [[nodiscard]] double getHp() const;
    [[nodiscard]] EnemyType getEnemyType() const;
    [[nodiscard]] EnemyPersonality getEnemyPersonality() const;

#pragma endregion

#pragma region Actions

    /**
     * @brief Sets and normalizes action weights.
     *
     * @param weights New action weights.
     */
    void setActionWeights(const EnemyActionWeights& weights);

    /**
     * @brief Returns current action weights.
     *
     * @return const EnemyActionWeights& Action weights.
     */
    [[nodiscard]] const EnemyActionWeights& getActionWeights() const;

#pragma endregion

    [[nodiscard]] std::string typeName() const override {
        return "EnemyEntity";
    }

  private:
#pragma region Territory

    std::unordered_set<HexCell*> _territory;
    std::unordered_set<HexCell*> _blockedEnemyCells;

#pragma endregion

#pragma region Attributes

    double _hp = 1;
    EnemyType _type = EnemyType::Normal;
    EnemyPersonality _personality = EnemyPersonality::Balanced;

#pragma endregion

#pragma region Actions

    EnemyActionWeights _actionWeights;

#pragma endregion
};

} // namespace game

#endif // GAME_ENEMY_ENTITY_H