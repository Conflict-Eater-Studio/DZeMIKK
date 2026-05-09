#include "scripts/world/world.h"

#include "ecs/components/collider.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "map/PlayerEntity.h"
#include "scripts/world/worldHex.h"

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
    _grid = HexGrid();
    auto idx =
        _grid.makeChunk({.steps = _randSteps(_rng, std::uniform_int_distribution<int>::param_type(
                                                       std::get<0>(_chunkConfigs.at(0)),
                                                       std::get<1>(_chunkConfigs.at(0)))),
                         .holeChance = 0.25F});

    for (auto& chunkConfig : _chunkConfigs) {
        idx = _grid.makeChunk(
            idx, HexCoord::Direction::R0,
            {.steps = _randSteps(_rng, std::uniform_int_distribution<int>::param_type(
                                           std::get<0>(chunkConfig), std::get<1>(chunkConfig))),
             .holeChance = 0.25F});

        for (const auto& dir : std::get<2>(chunkConfig)) {
            _grid.makeChunk(
                idx, dir,
                {.steps = _randSteps(_rng, std::uniform_int_distribution<int>::param_type(4, 8)),
                 .holeChance = 0.25F});
        }
    }

    auto* scene = _owner->getScene();
    for (auto* cell : _grid.getHexes()) {
        auto* obj = scene->createGameObject(
            std::format("Hex {} {}", cell->getCoord().q(), cell->getCoord().r()), _owner);
        auto height = _perlin.noise(static_cast<float>(cell->getCoord().q()) * 0.1F,
                                    static_cast<float>(cell->getCoord().r()) * 0.1F) *
                      2.0F;
        cell->getCoord().setHeight(height);
        auto worldPos = cell->getCoord().toWorldPosition(.65F, 0.0F);
        obj->transform()->setPosition(worldPos);
        obj->transform()->setScale({1.0F, 1.0F, 1.0F});
        obj->transform()->setRotation(
            glm::angleAxis(glm::radians(-90.0F), glm::vec3{1.0F, 0.0F, 0.0F}));
        auto* meshRenderer = obj->addComponent<dzemikk::MeshRenderer>();
        meshRenderer->setModel(_model);
        meshRenderer->setMaterial(0, _material);
        meshRenderer->setTransform(obj->transform());
        meshRenderer->setColor(glm::vec4(1.0F, 0.5F, 0.2F, 1.0F));
        auto* worldHex = obj->addComponent<WorldHex>();
        worldHex->setHexCell(cell);

        auto* collider = obj->addComponent<dzemikk::Collider>();
        collider->setModel(_model);
        collider->setTransform(obj->transform());
    }

    HexChunk* chunk = _grid.update(_grid.getChunks().at(0).getId());
    if (chunk == nullptr) {
        return;
    }
    _player->tryMove(&chunk->getHexes().at({2, 2}));
}

void World::setPlayer(PlayerEntity* playerEntity) {
    if (playerEntity == nullptr) {
        return;
    }

    _player = playerEntity;
}
} // namespace game
