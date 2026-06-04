#pragma once

#include "ecs/components/monobehaviour.h"
#include "map/HexGrid.h"

#include <random>
#include "enemyTypes.h"

namespace dzemikk {
class AssetManager;
}

namespace game {

class World;
class TerritoryPattern;

class EnemyManager : public dzemikk::MonoBehaviour {
  public:
    struct EnemySpawnConfig {
        EnemyPersonality personality;
        EnemyType type;
        int count;
        int hp;
        std::string territoryPattern;
    };

    EnemyManager(unsigned int seed = 1);

    void setWorld(World* world);
    void setAssetManager(dzemikk::AssetManager* assetManager);
    void setSpawnConfig(const boost::uuids::uuid& chunkId,
                        const std::vector<EnemySpawnConfig>& config);

    void spawnEnemiesPerChunk();

    [[nodiscard]] std::string typeName() const override {
        return "EnemyManager";
    }

  private:
    World* _world = nullptr;
    dzemikk::AssetManager* _assetManager = nullptr;
    std::mt19937 _rng;

    std::unordered_map<boost::uuids::uuid, std::vector<EnemySpawnConfig>> _spawnRules;

    void spawnEnemy(HexChunk::HexCellPtr cell, const EnemySpawnConfig& cfg);
    void assignTerritory(EnemyEntity* enemy, HexChunk::HexCellPtr centerCell,
                                       const TerritoryPattern& pattern);
    HexChunk* findChunkForCoord(const HexCoord& coord);
    bool canPlacePattern(HexCoord center, const TerritoryPattern& pattern);
};

} // namespace game