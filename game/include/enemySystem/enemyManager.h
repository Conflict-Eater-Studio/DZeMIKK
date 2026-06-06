#pragma once

#include "ecs/components/monobehaviour.h"
#include "enemyTypes.h"
#include "map/HexGrid.h"

#include <random>

namespace dzemikk {
class AssetManager;
}

namespace game {

class World;
class TerritoryPattern;

class EnemyManager : public dzemikk::MonoBehaviour {
  public:
    EnemyManager(unsigned int seed = 1);

    void setWorld(World* world);
    void setAssetManager(dzemikk::AssetManager* assetManager);
    void setSpawnConfig(const boost::uuids::uuid& chunkId,
                        const std::vector<EnemySpawnConfig>& config);

    void spawnEnemiesPerChunk();
    EnemyEntity* getEnemyByCell(HexCell* cell) const;

    [[nodiscard]] std::string typeName() const override {
        return "EnemyManager";
    }

  private:
    World* _world = nullptr;
    dzemikk::AssetManager* _assetManager = nullptr;
    std::mt19937 _rng;

    std::unordered_map<boost::uuids::uuid, std::vector<EnemySpawnConfig>> _spawnRules;
    std::unordered_map<HexCoord, EnemyEntity*> _cellToEnemy;

    void spawnEnemy(HexChunk::HexCellPtr cell, const EnemySpawnConfig& cfg,
                    const boost::uuids::uuid& spawnChunkId);
    void assignTerritory(EnemyEntity* enemy, HexChunk::HexCellPtr centerCell,
                         const TerritoryPattern& pattern);
    bool canPlacePattern(HexCoord center, const TerritoryPattern& pattern);
};

} // namespace game
