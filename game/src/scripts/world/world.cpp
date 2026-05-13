#include "scripts/world/world.h"

#include "boost/uuid/detail/nil_uuid.hpp"
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

World::World(int seed) : _rng(seed) {}

void World::start() {
    _grid = HexGrid();
}

void World::update(double dt) {
    for (auto* trs : _hexTransforms) {
        auto* cell = trs->getOwner()->getComponent<game::WorldHex>();
        if (cell->getHexCell()->isDirty()) {
            auto color = glm::vec4(1.0F);
            if (cell->getHexCell()->getGenState() == HexCell::GenState::Normal) {
                color = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
            } else if (cell->getHexCell()->getGenState() == HexCell::GenState::Blocked) {
                color = glm::vec4(0.2F, 0.2F, 0.2F, 1.0F);
            } else if (cell->getHexCell()->getGenState() == HexCell::GenState::Protected) {
                color = glm::vec4(0.2F, 0.5F, 1.0F, 1.0F);
            }
            cell->getOwner()->getComponent<dzemikk::MeshRenderer>()->setColor(color);
            cell->getHexCell()->setDirty(false);
        }
    }
}

boost::uuids::uuid World::addChunk(const HexChunk::Config& config) {
    auto* scene = _owner->getScene();
    auto chunkId = _grid.makeChunk(config);
    for (const auto& hex : _grid.getChunks().at(chunkId)->getHexes()) {
        auto cell = hex.second;

        if (cell->getGenState() == HexCell::GenState::Blocked) {
            continue;
        }

        auto* obj = scene->createGameObject(
            std::format("Hex {} {}", cell->getCoord().q(), cell->getCoord().r()), _owner);
        _hexTransforms.insert(obj->transform());
        auto height = _perlin.noise(static_cast<float>(cell->getCoord().q()) * 0.1F,
                                    static_cast<float>(cell->getCoord().r()) * 0.1F) *
                      2.0F;
        cell->getCoord().setHeight(height);
        auto worldPos = cell->getCoord().toWorldPosition(1.0F, 0.0F);
        obj->transform()->setPosition(worldPos);
        obj->transform()->setScale({1.0F, 1.0F, 1.0F});
        obj->transform()->setRotation(
            glm::angleAxis(glm::radians(-90.0F), glm::vec3{1.0F, 0.0F, 0.0F}));
        auto* meshRenderer = obj->addComponent<dzemikk::MeshRenderer>();
        meshRenderer->setModel(_model);
        switch (cell->getGenState()) {
        case HexCell::GenState::Blocked:
            meshRenderer->setMaterial(0, _material);
            meshRenderer->setColor(glm::vec4(0.2F, 0.2F, 0.2F, 1.0F));
            break;
        case HexCell::GenState::Protected:
            meshRenderer->setMaterial(0, _material);
            meshRenderer->setColor(glm::vec4(0.2F, 0.5F, 1.0F, 1.0F));
            break;
        case HexCell::GenState::Normal:
            meshRenderer->setMaterial(0, _material);
            meshRenderer->setColor(glm::vec4(1.0F, 1.0F, 1.0F, 1.0F));
            break;
        }
        meshRenderer->setTransform(obj->transform());
        auto* worldHex = obj->addComponent<WorldHex>();
        worldHex->setHexCell(cell);

        auto* collider = obj->addComponent<dzemikk::Collider>();
        collider->setModel(_model);
        collider->setTransform(obj->transform());
    }

    return chunkId;
}

void World::setPlayer(PlayerEntity* playerEntity) {
    if (playerEntity == nullptr) {
        return;
    }

    _player = playerEntity;
}
} // namespace game
