#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include <memory>
#pragma once
#include "boost/uuid/uuid_io.hpp"
#include "ecs/components/monobehaviour.h"
#include "ecs/components/transform.h"
#include "enemySystem/enemyEntity.h"
#include "map/HexGrid.h"
#include "map/HexPattern.h"
#include "renderer/material.h"
#include "renderer/model.h"
#include "utils/perlin.h"

#include <nlohmann/json.hpp>
#include <random>
#include <unordered_set>

class Game;

namespace dzemikk {
class AssetManager;
}

namespace game {
class PlayerEntity;
class EnemyEntity;

class World : public dzemikk::MonoBehaviour {
  public:
    using Base = dzemikk::MonoBehaviour;

    struct ChunkData {
        boost::uuids::uuid parentPersistantId;
        boost::uuids::uuid childPersistantId;
        std::string name;
        int steps;
        std::string generatorId{"default"};
        HexCoord::Direction dirFromParent;
        std::optional<HexPattern> unlockPattern = std::nullopt;
    };

    struct WorldData {
        unsigned int seed;
        std::vector<ChunkData> chunks;
    };

    World(unsigned int seed);

    void start() override {};
    void update(double dt) override;
    void lateUpdate(double dt) override {};
    void fixedUpdate(double dt) override {};
    void onDestroy() override {};

    [[nodiscard]] std::string typeName() const override {
        return "World";
    }

    [[nodiscard]] HexGrid* getGrid() {
        return &_grid;
    }
    [[nodiscard]] PlayerEntity* getPlayer() {
        return _player;
    }
    [[nodiscard]] WorldData& getWorldDefinition() {
        return _worldData;
    }

    void setPlayer(PlayerEntity* playerEntity);

    boost::uuids::uuid addChunk(const ChunkData& config);
    void renderChunk(boost::uuids::uuid id);

    nlohmann::json save();
    void load(const nlohmann::json& def);

    void registerGenerator(const std::string& id,
                           std::function<float(int step, int maxSteps)> generator) {
        _generators[id] = std::move(generator);
    }
    void unregisterGenerator(const std::string& id) {
        _generators.erase(id);
    }

    void ensureHexExists(const std::shared_ptr<HexCell>& cell);
    [[nodiscard]] bool hasHexVisual(const HexCoord& coord) const;

    bool reserveTerritory(const HexCoord& coord) {
        return _reservedTerritory.insert(coord).second;
    }

    [[nodiscard]] bool isTerritoryReserved(const HexCoord& coord) const {
        return _reservedTerritory.contains(coord);
    }

    dzemikk::Transform* getHexTransformByCell(HexCell cell);

    void setGame(Game* game) {
        _game = game;
    }
    void setAssetManager(dzemikk::AssetManager* assetManager) {
        _assetManager = assetManager;
    }

  private:
    void spawnHexVisual(const std::shared_ptr<HexCell>& cell);

    std::unordered_map<std::string, std::function<float(int step, int maxSteps)>> _generators;

    Game* _game{nullptr};
    dzemikk::AssetManager* _assetManager{nullptr};
    WorldData _worldData;
    HexGrid _grid;
    Perlin _perlin;
    std::mt19937 _rng;
    std::uniform_int_distribution<int> _randSteps;
    std::unordered_set<dzemikk::Transform*> _hexTransforms;
    std::unordered_set<HexCoord> _spawnedHexes;
    std::unordered_set<HexCoord> _reservedTerritory;

    PlayerEntity* _player{nullptr};

    // --- Materials and Models
    dzemikk::AssetHandle<dzemikk::Model> _hexModel;
    std::shared_ptr<dzemikk::Material> _hexMaterial;
};

} // namespace game

#endif // GAME_WORLD_H
