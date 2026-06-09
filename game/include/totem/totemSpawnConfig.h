#pragma once
#include <string>

namespace game {

struct TotemSpawnConfig {
    int count = 1;

    std::string prefabPath = "prefabs/totem/totem_container.prefab";
};

} // namespace game