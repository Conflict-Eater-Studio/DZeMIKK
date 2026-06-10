#ifndef GAME_TOTEM_SPAWN_CONFIG_H
#define GAME_TOTEM_SPAWN_CONFIG_H

#include "map/HexPattern.h"

#include <string>
#include <vector>

namespace game {

/**
 * @brief Configuration describing how totems are spawned in the world.
 *
 * Defines the number of totems to generate, the pattern granted by the
 * totem, and the prefab resources used to construct its visual appearance.
 */
struct TotemSpawnConfig {

    /**
     * @brief Number of totems to spawn.
     */
    int count = 1;

    /**
     * @brief Pattern granted or associated with the totem.
     */
    HexPattern pattern;

    /**
     * @brief Prefab used as the root container for the totem.
     */
    std::string prefabPath = "prefabs/totem/totem_container.prefab";

    /**
     * @brief Pool of visual segment prefabs used to assemble the totem.
     */
    std::vector<std::string> segmentPool = {
        "prefabs/totem/totem1.prefab", "prefabs/totem/totem2.prefab",
        "prefabs/totem/totem3.prefab", "prefabs/totem/totem4.prefab",
        "prefabs/totem/totem5.prefab", "prefabs/totem/totem6.prefab"};
};

} // namespace game

#endif