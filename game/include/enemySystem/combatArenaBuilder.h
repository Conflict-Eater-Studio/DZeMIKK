#ifndef GAME_COMBAT_ARENA_BUILDER_H
#define GAME_COMBAT_ARENA_BUILDER_H

#include "map/Entity.h"
#include "map/HexCell.h"
#include "map/HexCoord.h"

#include <memory>
#include <vector>

namespace game {
class World;
class EnemyEntity;
class PlayerEntity;
class HexGrid;
class HexCell;
class HexChunk;
} // namespace game

/**
 * @brief Result of combat arena generation.
 *
 * Contains all cells created during arena construction and
 * a pointer to the arena center cell.
 */
struct CombatArenaResult {

    /**
     * @brief Cells created while building the combat arena.
     */
    std::vector<game::HexCell*> createdCells;

    /**
     * @brief Center cell of the generated combat arena.
     */
    game::Entity::HexCellPtr centerCell = nullptr;
};

/**
 * @brief Builds a temporary combat arena from player and enemy territories.
 *
 * Creates a dedicated combat area on the hex grid, copies relevant
 * territory data, creates missing cells, and returns information
 * about the generated arena.
 */
class CombatArenaBuilder {
  public:
    /**
     * @brief Creates a combat arena for the given combatants.
     *
     * @param enemy Enemy participating in combat.
     * @param player Player participating in combat.
     * @param grid Grid used for cell creation and lookup.
     * @param world World containing arena chunks and cells.
     *
     * @return Information about the generated combat arena.
     */
    static CombatArenaResult build(game::EnemyEntity* enemy, game::PlayerEntity* player,
                            game::HexGrid* grid, game::World* world);

  private:
    /**
     * @brief Copies territory cell coordinates.
     *
     * @param territory Territory cells to copy.
     * @return Coordinates of all territory cells.
     */
    static std::vector<game::HexCoord> copyTerritory(const std::vector<game::HexCell*>& territory);

    /**
     * @brief Shifts territory coordinates into arena space.
     *
     * @param coords Original territory coordinates.
     * @return Shifted coordinates.
     */
    static std::vector<game::HexCoord> shiftTerritory(const std::vector<game::HexCoord>& coords);

    /**
     * @brief Ensures a cell exists at the specified coordinate.
     *
     * Creates the cell if it does not already exist.
     *
     * @param coord Target coordinate.
     * @param grid Grid containing the cell.
     * @param world World owning the cell.
     * @param chunk Chunk that should contain the cell.
     * @param type Type assigned to the cell.
     *
     * @return Pointer to the existing or newly created cell.
     */
    static game::Entity::HexCellPtr ensureCellExists(const game::HexCoord& coord, game::HexGrid* grid,
                                              game::World* world, game::HexChunk* chunk,
                                              game::HexCell::Type type);

    /**
     * @brief Calculates the center coordinate of a territory.
     *
     * @param coords Territory coordinates.
     * @return Center coordinate.
     */
    static game::HexCoord calculateCenter(const std::vector<game::HexCoord>& coords);

    /**
     * @brief Returns any available chunk from the world.
     *
     * Used as a destination chunk for arena cell creation.
     *
     * @param world World containing chunks.
     * @return Pointer to a chunk or nullptr if none exist.
     */
    static game::HexChunk* getAnyChunk(game::World* world);
};

#endif