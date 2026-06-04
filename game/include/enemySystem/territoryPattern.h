#pragma once

#include "map/HexCoord.h"

#include <string>
#include <vector>

namespace game {

struct TerritoryPattern {
    std::string name;
    std::vector<HexCoord> offsets;
};

} // namespace game