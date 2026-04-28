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

    void setModel(dzemikk::Model* model) {
        _model = model;
    }
    void setMaterial(dzemikk::Material* material) {
        _material = material;
    }

  private:
    Grid _grid;
    Perlin _perlin{1};
    std::mt19937 _rng;
    std::uniform_int_distribution<int> _randSteps;
    std::vector<std::tuple<int, int, std::vector<HexCoord::Direction>>> _chunkConfigs;
    dzemikk::Model* _model{nullptr};
    dzemikk::Material* _material{nullptr};
};
} // namespace game

#endif // GAME_WORLD_H
