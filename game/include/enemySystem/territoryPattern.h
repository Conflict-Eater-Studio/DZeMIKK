#ifndef GAME_TERRITORY_PATTERN_H
#define GAME_TERRITORY_PATTERN_H

#include "map/HexCoord.h"

#include <string>
#include <vector>

namespace game {

/**
 * @brief Defines a territory shape using offsets relative to a center cell.
 *
 * Territory patterns are used to determine which cells belong to an enemy
 * territory when assigning ownership around a spawn location.
 */
struct TerritoryPattern {
    std::string name;
    std::vector<HexCoord> offsets;
};

} // namespace game

#endif