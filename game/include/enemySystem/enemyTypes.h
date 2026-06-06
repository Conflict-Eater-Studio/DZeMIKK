#pragma once
#include "boost/uuid/uuid.hpp"

namespace game {

enum class EnemyPersonality { Aggressive, Defensive, Balanced };
enum class EnemyType { Normal, Special };

struct EnemySpawnConfig {
    EnemyPersonality personality;
    EnemyType type;
    int count;
    int hp;
    std::string territoryPattern;
    std::vector<boost::uuids::uuid> blocksChunks;
};

} // namespace game
