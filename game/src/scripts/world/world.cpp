#include "scripts/world/world.h"

#include "ecs/components/meshRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <format>
#include <memory>
#include <random>
#include <vector>

namespace game {
using dzemikk::Model;

World::World(int seed, int chunkMinSteps, int chunkMaxSteps, int chunkCound) : _rng(seed) {
    for (int i = 0; i < chunkCound; ++i) {
        _chunkConfigs.emplace_back(chunkMinSteps, chunkMaxSteps,
                                   std::vector<HexCoord::Direction>{});
    }
}

World::World(int seed,
             std::vector<std::tuple<int, int, std::vector<HexCoord::Direction>>> chunkConfigs)
    : _rng(seed), _chunkConfigs(std::move(chunkConfigs)) {}

void World::start() {
    _grid = Grid();
    auto idx = _grid.makeChunk(
        HexCoord{0, 0}, {.steps = _randSteps(_rng, std::uniform_int_distribution<int>::param_type(
                                                       std::get<0>(_chunkConfigs.at(0)),
                                                       std::get<1>(_chunkConfigs.at(0)))),
                         .holeChance = 0.25F});
    for (auto& chunkConfig : _chunkConfigs) {
        idx = _grid.makeChunk(
            idx.value(), HexCoord::Direction::R0,
            {.steps = _randSteps(_rng, std::uniform_int_distribution<int>::param_type(
                                           std::get<0>(chunkConfig), std::get<1>(chunkConfig))),
             .holeChance = 0.25F});

        for (const auto& dir : std::get<2>(chunkConfig)) {
            _grid.makeChunk(
                idx.value(), dir,
                {.steps = _randSteps(_rng, std::uniform_int_distribution<int>::param_type(4, 8)),
                 .holeChance = 0.25F});
        }
    }

    auto* scene = _owner->getScene();
    for (auto cell : _grid.getHexes()) {
        auto* obj = scene->createGameObject(
            std::format("Hex {} {}", cell.coord.q(), cell.coord.r()), _owner);
        auto height = _perlin.noise(static_cast<float>(cell.coord.q()) * 0.1F,
                                    static_cast<float>(cell.coord.r()) * 0.1F) *
                      2.0F;
        cell.coord.setHeight(height);
        auto worldPos = cell.coord.toWorldPosition(std::numbers::sqrt3_v<float> / 2.0F, 0.0F);
        obj->transform()->setPosition(worldPos);
        obj->transform()->setScale({1.0F, 1.0F, 1.0F});
        obj->transform()->setRotation(
            glm::angleAxis(glm::radians(-90.0F), glm::vec3{1.0F, 0.0F, 0.0F}));
        auto* meshRenderer = obj->addComponent<dzemikk::MeshRenderer>();
        meshRenderer->setModel(_model);
        meshRenderer->setMaterial(0, _material);
        meshRenderer->setTransform(obj->transform());

        auto* entityGO = scene->createGameObject(_owner);
        auto* entityRenderer = entityGO->addComponent<dzemikk::MeshRenderer>();

        if (cell.onHex.second == GridCell::OnHex::Enemy) {
            entityRenderer->setModel(_enemyModel);
        } else if (cell.onHex.second == GridCell::OnHex::Resource) {
            entityRenderer->setModel(_resourceModel);
        } else {
            continue;
        }

        entityRenderer->setMaterial(0, _material2);
        entityRenderer->setTransform(entityGO->transform());
        entityGO->transform()->setPosition(worldPos + glm::vec3{0.0F, 2.0F, 0.0F});
    }
}
} // namespace game
