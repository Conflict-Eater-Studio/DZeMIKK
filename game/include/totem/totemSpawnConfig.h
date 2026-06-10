#pragma once
#include <string>
#include <vector>

#include "map/HexPattern.h"

namespace game {

struct TotemSpawnConfig {
    int count = 1;
    HexPattern pattern;

    std::string prefabPath = "prefabs/totem/totem_container.prefab";

    std::vector<std::string> segmentPool = {
        "prefabs/totem/totem1.prefab", "prefabs/totem/totem2.prefab",
        "prefabs/totem/totem3.prefab", "prefabs/totem/totem4.prefab",
        "prefabs/totem/totem5.prefab", "prefabs/totem/totem6.prefab"};
};

} // namespace game