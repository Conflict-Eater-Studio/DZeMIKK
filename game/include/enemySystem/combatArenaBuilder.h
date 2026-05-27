#pragma once

#include <memory>
#include <vector>
#include <map/HexCoord.h>
#include <map/HexCell.h>
#include <map/Entity.h>

namespace game {
class World;
class EnemyEntity;
class PlayerEntity;
class HexGrid;
class HexCell;
class HexChunk;
} // namespace game

struct CombatArenaResult {
    std::vector<game::HexCell*> createdCells;
    game::Entity::HexCellPtr centerCell = nullptr;
};

class CombatArenaBuilder {
  public:
    CombatArenaResult build(game::EnemyEntity* enemy, game::PlayerEntity* player, game::HexGrid* grid,
                            game::World* world);

  private:
    std::vector<game::HexCoord> copyTerritory(const std::vector<game::HexCell*>& territory);

    std::vector<game::HexCoord> shiftTerritory(const std::vector<game::HexCoord>& coords);

    std::unordered_set<game::HexCoord> buildBlockedSet(const std::vector<game::HexCell*>& territory);

    game::Entity::HexCellPtr ensureCellExists(const game::HexCoord& coord, game::HexGrid* grid,
                                               game::World* world,
                              game::HexChunk* chunk, game::HexCell::Type type);

    game::HexCoord calculateCenter(const std::vector<game::HexCoord>& coords);

    game::HexChunk* getAnyChunk(game::World* world);
};