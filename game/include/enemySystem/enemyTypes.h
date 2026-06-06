#ifndef GAME_ENEMY_TYPES_H
#define GAME_ENEMY_TYPES_H

#include <cstdint>
#include "boost/uuid/uuid.hpp"

namespace game {

/**
 * @brief Defines enemy behavioral tendencies.
 */
enum class EnemyPersonality : std::uint8_t { Aggressive, Defensive, Balanced };

/**
 * @brief Defines available enemy categories.
 */
enum class EnemyType : std::uint8_t { Normal, Special };

struct EnemySpawnConfig {
    EnemyPersonality personality;
    EnemyType type;
    int count;
    int hp;
    std::string territoryPattern;
    std::vector<boost::uuids::uuid> blocksChunks;
};

} // namespace game

#endif 

