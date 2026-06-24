#ifndef GAME_ENEMY_TYPES_H
#define GAME_ENEMY_TYPES_H

#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/string_generator.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_io.hpp"

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace game {

/**
 * @brief Defines enemy behavioral tendencies.
 */
enum class EnemyPersonality : std::uint8_t { Aggressive, Defensive, Balanced };

/**
 * @brief Defines available enemy categories.
 */
enum class EnemyType : std::uint8_t { Normal, Special, Boss };

struct EnemySpawnConfig {
    boost::uuids::uuid persistantId = boost::uuids::random_generator()();
    boost::uuids::uuid chunkId;
    EnemyPersonality personality;
    EnemyType type;
    int count;
    int hp;
    std::string territoryPattern;
    std::vector<boost::uuids::uuid> blocksChunks;
    std::string name;
};

// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& j, const EnemySpawnConfig& config) {
    j = nlohmann::json{{"persistantId", boost::uuids::to_string(config.persistantId)},
                       {"personality", static_cast<int>(config.personality)},
                       {"type", static_cast<int>(config.type)},
                       {"count", config.count},
                       {"hp", config.hp},
                       {"territoryPattern", config.territoryPattern},
                       {"blocksChunks", nlohmann::json::array()},
                        {"name", config.name}};


    for (const auto& chunkId : config.blocksChunks) {
        j["blocksChunks"].push_back(boost::uuids::to_string(chunkId));
    }
}

inline void from_json(const nlohmann::json& j, EnemySpawnConfig& config) {
    if (!j.contains("personality") || !j.contains("type") || !j.contains("count") ||
        !j.contains("hp") || !j.contains("territoryPattern") || !j.contains("blocksChunks")
        || !j.contains("name")) {
        throw std::runtime_error("Invalid JSON for EnemySpawnConfig");
    }
    if (j.contains("persistantId")) {
        config.persistantId =
            boost::uuids::string_generator()(j.at("persistantId").get<std::string>());
    }
    config.personality = static_cast<EnemyPersonality>(j["personality"].get<int>());
    config.type = static_cast<EnemyType>(j["type"].get<int>());
    j.at("count").get_to(config.count);
    j.at("hp").get_to(config.hp);
    j.at("territoryPattern").get_to(config.territoryPattern);
    j.at("name").get_to(config.name);
    for (const auto& chunkIdStr : j["blocksChunks"]) {
        config.blocksChunks.push_back(
            boost::uuids::string_generator()(chunkIdStr.get<std::string>()));
    }
}
// NOLINTEND(readability-identifier-naming)

} // namespace game

#endif
