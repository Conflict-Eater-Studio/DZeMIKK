#ifndef GAME_ITEM_MANAGER_H
#define GAME_ITEM_MANAGER_H

#pragma once
#include "ecs/components/monobehaviour.h"
#include "map/HexChunk.h"
#include "map/ItemEntity.h"

#include <random>

class Game;

namespace dzemikk {
class AssetManager;
}

namespace game {

class World;

class ItemManager : public dzemikk::MonoBehaviour {
  public:
    ItemManager(unsigned int seed = 1);

    void setWorld(World* world);
    void setAssetManager(dzemikk::AssetManager* assetManager);
    void setGame(Game* game);

    void addItem(const boost::uuids::uuid& chunkId, ItemSpawnConfig config);
    void addItem(const boost::uuids::uuid& chunkId, ItemSpawnConfig config, const HexCoord& coord);

    void update(double dt) override;

    [[nodiscard]] std::string typeName() const override {
        return "ItemManager";
    }

    [[nodiscard]] nlohmann::json saveState() const;
    void loadState(const nlohmann::json& j);
    void clear();

  private:
    void spawnItem(const boost::uuids::uuid& chunkId, const HexChunk::HexCellPtr& cell,
                   const ItemSpawnConfig& cfg);

    static std::vector<HexChunk::HexCellPtr> collectAvailableCells(HexChunk* chunk);

    World* _world = nullptr;
    dzemikk::AssetManager* _assetManager = nullptr;
    Game* _game = nullptr;

    std::mt19937 _rng;

    std::unordered_map<boost::uuids::uuid, std::vector<ItemSpawnConfig>> _spawnRules;
    std::unordered_map<boost::uuids::uuid, std::vector<ItemEntity*>> _spawnedItems;
};

} // namespace game

#endif // GAME_ITEM_MANAGER_H
