#pragma once

#include "ecs/components/monobehaviour.h"
#include "totemSpawnConfig.h"

#include "map/HexChunk.h"
#include <random>

class Game;

namespace dzemikk {
class AssetManager;
}

namespace game {

class World;
class TotemEntity;

class TotemManager : public dzemikk::MonoBehaviour {
  public:
    TotemManager(unsigned int seed = 1);

    void setWorld(World* world);
    void setAssetManager(dzemikk::AssetManager* assetManager);
    void setGame(Game* game);

    void setSpawnConfig(const boost::uuids::uuid& chunkId,
                        const std::vector<TotemSpawnConfig>& config);

    void spawnTotemsPerChunk();

    [[nodiscard]]
    std::string typeName() const override {
        return "TotemManager";
    }

  private:
    void spawnTotem(HexChunk::HexCellPtr cell, const TotemSpawnConfig& cfg);

    static std::vector<HexChunk::HexCellPtr> collectAvailableCells(HexChunk* chunk);

    World* _world = nullptr;

    dzemikk::AssetManager* _assetManager = nullptr;
    Game* _game = nullptr;

    std::mt19937 _rng;

    std::unordered_map<boost::uuids::uuid, std::vector<TotemSpawnConfig>> _spawnRules;
};

} // namespace game