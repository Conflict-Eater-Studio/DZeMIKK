#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include "ecs/components/monobehaviour.h"
#include "map/grid.h"
#include "renderer/material.h"
#include "renderer/model.h"
#include "utils/perlin.h"

#include <random>
#include <tuple>

namespace game {
class World : public dzemikk::MonoBehaviour {
  public:
    World(int seed, int chunkMinSteps = 4, int chunkMaxSteps = 10, int chunkCound = 10);
    World(int seed,
          std::vector<std::tuple<int, int, std::vector<HexCoord::Direction>>> chunkConfigs);

    void start() override;
    void update(double dt) override {};
    void lateUpdate() override {};
    void fixedUpdate(double dt) override {};
    void onDestroy() override {};

    [[nodiscard]] std::string typeName() const override {
        return "World";
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

  private:
    Grid _grid;
    Perlin _perlin{1};
    std::mt19937 _rng;
    std::uniform_int_distribution<int> _randSteps;
    std::vector<std::tuple<int, int, std::vector<HexCoord::Direction>>> _chunkConfigs;
    dzemikk::AssetHandle<dzemikk::Model> _model;
    dzemikk::AssetHandle<dzemikk::Model>  _enemyModel;
    dzemikk::AssetHandle<dzemikk::Model>  _resourceModel;
    std::shared_ptr<dzemikk::Material> _material;
    std::shared_ptr<dzemikk::Material> _material2;
};
} // namespace game

#endif // GAME_WORLD_H
