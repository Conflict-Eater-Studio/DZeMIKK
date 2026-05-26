#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/string_generator.hpp"
#include "boost/uuid/uuid_io.hpp"
#pragma once

#include "ecs/components/monobehaviour.h"
#include "ecs/components/transform.h"
#include "ecs/serialize/serializedRef.h"
#include "map/HexGrid.h"
#include "renderer/material.h"
#include "renderer/model.h"
#include "utils/perlin.h"

#include <nlohmann/json.hpp>
#include <random>
#include <tuple>
#include <unordered_set>

namespace game {
class PlayerEntity;
class World : public dzemikk::MonoBehaviour {
  public:
    struct ChunkDefinition {
        boost::uuids::uuid parentChunkId;
        boost::uuids::uuid chunkId;
        int steps;
        std::string generatorId{"default"};
        HexCoord::Direction dirFromParent;
    };

    struct WorldDefinition {
        unsigned int seed;
        std::vector<ChunkDefinition> chunks;
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

    void setModel(const dzemikk::AssetHandle<dzemikk::Model>& model) {
        _model = model;
    }
    void setEnemyModel(const dzemikk::AssetHandle<dzemikk::Model>& model) {
        _enemyModel = model;
    }
    void setResourceModel(const dzemikk::AssetHandle<dzemikk::Model>& model) {
        _resourceModel = model;
    }
    void setMaterial(std::shared_ptr<dzemikk::Material> material) {
        _material = material;
    }
    void setMaterial2(std::shared_ptr<dzemikk::Material> material) {
        _material2 = material;
    }
    void setPlayer(PlayerEntity* playerEntity);

    boost::uuids::uuid addChunk(const ChunkDefinition& config);
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

  private:
    void spawnHexVisual(const std::shared_ptr<HexCell>& cell);

    std::unordered_map<std::string, std::function<float(int step, int maxSteps)>> _generators;

    WorldDefinition _worldDefinition;
    HexGrid _grid;
    Perlin _perlin;
    std::mt19937 _rng;
    std::uniform_int_distribution<int> _randSteps;
    dzemikk::AssetHandle<dzemikk::Model> _model;
    dzemikk::AssetHandle<dzemikk::Model> _enemyModel;
    dzemikk::AssetHandle<dzemikk::Model> _resourceModel;
    std::shared_ptr<dzemikk::Material> _material;
    std::shared_ptr<dzemikk::Material> _material2;
    std::unordered_set<dzemikk::Transform*> _hexTransforms;
    std::unordered_set<HexCoord> _spawnedHexes;

    PlayerEntity* _player{nullptr};
};

// NOLINTBEGIN(readability-identifier-naming)
// --- JSON Serialization for ChunkDefinition ---
inline void to_json(nlohmann::json& j, const World::ChunkDefinition& def) {
    j = nlohmann::json{
        {"parentChunkId", boost::uuids::to_string(def.parentChunkId)},
        {"chunkId", boost::uuids::to_string(def.chunkId)},
        {"steps", def.steps},
        {"generatorId", def.generatorId},
        {"dirFromParent", def.dirFromParent},
    };
}

inline void from_json(const nlohmann::json& j, World::ChunkDefinition& def) {
    if (!j.contains("parentChunkId") || !j.contains("steps") || !j.contains("generatorId") ||
        !j.contains("dirFromParent") || !j.contains("chunkId")) {
        throw std::runtime_error("Invalid JSON for World::ChunkDefinition");
    }

    def.parentChunkId = boost::uuids::string_generator()(j["parentChunkId"].get<std::string>());
    def.chunkId = boost::uuids::string_generator()(j["chunkId"].get<std::string>());
    j.at("steps").get_to(def.steps);
    j.at("generatorId").get_to(def.generatorId);
    j.at("dirFromParent").get_to(def.dirFromParent);
}

// --- JSON Serialization for WorldDefinition ---
inline void to_json(nlohmann::json& j, const World::WorldDefinition& def) {
    j = nlohmann::json{
        {"seed", def.seed},
        {"chunks", def.chunks},
    };
}

inline void from_json(const nlohmann::json& j, World::WorldDefinition& def) {
    if (!j.contains("seed") || !j.contains("chunks")) {
        throw std::runtime_error("Invalid JSON for World::WorldDefinition");
    }
    j.at("seed").get_to(def.seed);
    j.at("chunks").get_to(def.chunks);
}
// NOLINTEND(readability-identifier-naming)
} // namespace game

#endif // GAME_WORLD_H
