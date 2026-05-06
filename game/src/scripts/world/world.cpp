#include "scripts/world/world.h"

#include "ecs/components/meshRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"

#include <format>
#include <memory>
#include <random>
#include <vector>
#include "renderer/material.h"
#include "renderer/shader.h"

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
        auto worldPos = cell.coord.toWorldPosition(.65F, 0.0F);
        obj->transform()->setPosition(worldPos);
        obj->transform()->setScale({1.0F, 1.0F, 1.0F});
        obj->transform()->setRotation(
            glm::angleAxis(glm::radians(-90.0F), glm::vec3{1.0F, 0.0F, 0.0F}));
        auto* meshRenderer = obj->addComponent<dzemikk::MeshRenderer>();
        meshRenderer->setModel(_model);
        meshRenderer->setMaterial(0, _material.get());
        meshRenderer->setTransform(obj->transform());
        meshRenderer->setColor(glm::vec4(1.0F, 0.5F, 0.2F, 1.0F));

        if (cell.onHex.second == GridCell::OnHex::Enemy) {
            auto* entityGO = scene->createGameObject(_owner);
            auto* entityRenderer = entityGO->addComponent<dzemikk::MeshRenderer>();
            entityRenderer->setModel(_enemyModel);
            entityRenderer->setMaterial(0, _material2.get());
            spdlog::info("{}", _material2->getShader()->getProgramID());
            entityRenderer->setTransform(entityGO->transform());
            entityRenderer->setColor(glm::vec4(0.0F, 0.5F, 1.0F, 1.0F));
            entityGO->transform()->setPosition(worldPos + glm::vec3{0.0F, 2.0F, 0.0F});
        } else if (cell.onHex.second == GridCell::OnHex::Resource) {
            auto* entityGO = scene->createGameObject(_owner);
            auto* entityRenderer = entityGO->addComponent<dzemikk::MeshRenderer>();
            entityRenderer->setModel(_resourceModel);
            entityRenderer->setMaterial(0, _material2.get());
            entityRenderer->setTransform(entityGO->transform());
            entityRenderer->setColor(glm::vec4(1.0F, 0.0F, 0.0F, 1.0F));
            entityGO->transform()->setPosition(worldPos + glm::vec3{0.0F, 2.0F, 0.0F});
        } else {
            continue;
        }

    }
}
} // namespace game
