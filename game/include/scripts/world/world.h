#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include "ecs/components/monobehaviour.h"
#include "ecs/components/transform.h"
#include "ecs/serialize/serializedRef.h"
#include "map/HexGrid.h"
#include "renderer/material.h"
#include "renderer/model.h"
#include "utils/perlin.h"

#include <random>
#include <tuple>
#include <unordered_set>

namespace game {
class PlayerEntity;
class World : public dzemikk::MonoBehaviour {
  public:
    World(int seed);

    void start() override;
    void update(double dt) override;
    void lateUpdate() override {};
    void fixedUpdate(double dt) override {};
    void onDestroy() override {};

    [[nodiscard]] std::string typeName() const override {
        return "World";
    }

    [[nodiscard]] HexGrid* getGrid() {
        return _grid.get();
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

    void renderChunk(boost::uuids::uuid id);

  private:
    void spawnHexVisual(const std::shared_ptr<HexCell>& cell);

    std::unique_ptr<HexGrid> _grid;
    Perlin _perlin{1};
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
} // namespace game

#endif // GAME_WORLD_H
