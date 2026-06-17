#ifndef GAME_TOTEM_SPAWN_CONFIG_H
#define GAME_TOTEM_SPAWN_CONFIG_H

#include "map/HexPattern.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace game {

/**
 * @brief Configuration describing how totems are spawned in the world.
 *
 * Defines the pattern granted by the totem, the prefab resources used to
 * construct its visual appearance, and persistent identification.
 */
struct TotemSpawnConfig {

    /**
     * @brief Unique persistent identifier for save/load.
     */
    boost::uuids::uuid persistantId = boost::uuids::random_generator()();

    /**
     * @brief Chunk identifier where the totem is placed.
     */
    boost::uuids::uuid chunkId;

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

    /**
     * @brief Indicates whether the totem has already been used.
     */
    bool used = false;
};

#pragma region JSON Serialization
// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& j, const TotemSpawnConfig& config) {
    j = nlohmann::json{{"persistantId", boost::uuids::to_string(config.persistantId)},
                       {"chunkId", boost::uuids::to_string(config.chunkId)},
                       {"pattern", config.pattern},
                       {"prefabPath", config.prefabPath},
                       {"segmentPool", config.segmentPool},
                       {"used", config.used}};
}

inline void from_json(const nlohmann::json& j, TotemSpawnConfig& config) {
    if (j.contains("persistantId")) {
        config.persistantId =
            boost::uuids::string_generator()(j.at("persistantId").get<std::string>());
    }
    if (j.contains("chunkId")) {
        config.chunkId =
            boost::uuids::string_generator()(j.at("chunkId").get<std::string>());
    }
    if (j.contains("pattern")) {
        config.pattern = j.at("pattern").get<HexPattern>();
    }
    if (j.contains("prefabPath")) {
        j.at("prefabPath").get_to(config.prefabPath);
    }
    if (j.contains("segmentPool")) {
        j.at("segmentPool").get_to(config.segmentPool);
    }
    if (j.contains("used")) {
        j.at("used").get_to(config.used);
    }
}
// NOLINTEND(readability-identifier-naming)
#pragma endregion

} // namespace game

#endif
