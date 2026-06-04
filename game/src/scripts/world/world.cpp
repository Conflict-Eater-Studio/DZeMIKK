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

namespace game {
World::World(unsigned int seed) : _rng(seed), _perlin(seed), _grid(seed) {
    _worldDefinition.seed = seed;
    _worldDefinition.chunks = {};
    _generators["default"] = [](int step, int maxSteps) {
        return 1.0F - (static_cast<float>(step) / static_cast<float>(maxSteps));
    };
}

void World::load(const nlohmann::json& def) {
    const WorldDefinition wd = def["world"].get<WorldDefinition>();
    _worldDefinition = wd;

    _rng = std::mt19937(_worldDefinition.seed);
    _perlin = Perlin(_worldDefinition.seed);

    for (auto* trs : _hexTransforms) {
        _owner->destroyChild(trs->getOwner());
    }
    _hexTransforms.clear();
    _spawnedHexes.clear();

    _grid = HexGrid(_worldDefinition.seed);

    const auto chunksToBuild = _worldDefinition.chunks;
    _worldDefinition.chunks.clear();
    for (const auto& chunkDef : chunksToBuild) {
        addChunk(chunkDef);
    }
}

nlohmann::json World::save() {
    return nlohmann::json{{"world", _worldDefinition}};
}

void World::update(double dt) {
    for (const auto& chunk : _grid.getChunks()) {
        for (const auto& [coord, cell] : chunk.second->getHexes()) {
            if (cell->getGenState() == HexCell::GenState::Blocked ||
                _spawnedHexes.contains(coord)) {
                continue;
            }
            spawnHexVisual(cell);
        }
    }

    for (auto* trs : _hexTransforms) {
        auto* cell = trs->getOwner()->getComponent<game::WorldHex>();
        if (cell->getHexCell()->isDirty()) {
            auto color = glm::vec4(1.0F);
            color = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);

            if (cell->getHexCell()->getType() == HexCell::Type::EnemyBattleHex) {
                color = glm::vec4(0.0F, 0.0F, 0.5F, 1.0F);
            }

            /*
            if (cell->getHexCell()->getGenState() == HexCell::GenState::Normal) {
                color = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
            } else if (cell->getHexCell()->getGenState() == HexCell::GenState::Blocked) {
                color = glm::vec4(0.2F, 0.2F, 0.2F, 1.0F);
            } else if (cell->getHexCell()->getGenState() == HexCell::GenState::Protected) {
                color = glm::vec4(0.2F, 0.5F, 1.0F, 1.0F);
            }
            */

            cell->getOwner()->getComponent<dzemikk::MeshRenderer>()->setColor(color);
            cell->getHexCell()->setDirty(false);
        }
    }
}

boost::uuids::uuid World::addChunk(const ChunkDefinition& config) {
    _worldDefinition.chunks.push_back(config);
    auto g = _generators.at(config.generatorId);

    if (g == nullptr) {
        throw std::runtime_error(
            std::format("Generator with id '{}' not found", config.generatorId));
    }

    auto id = _grid.makeChunk({.parentChunkId = config.parentChunkId,
                               .chunkId = config.chunkId,
                               .steps = config.steps,
                               .generator = g,
                               .dirFromParent = config.dirFromParent});

    _worldDefinition.chunks.back().chunkId = id;
    renderChunk(id);

    return id;
}

void World::renderChunk(boost::uuids::uuid id) {
    for (const auto& hex : _grid.getChunks().at(id)->getHexes()) {
        auto cell = hex.second;

        if (cell->getGenState() == HexCell::GenState::Blocked) {
            continue;
        }

        spawnHexVisual(cell);
    }
}

void World::ensureHexExists(const std::shared_ptr<HexCell>& cell) {
    if (!cell)
        return;

    spawnHexVisual(cell);
}

bool World::hasHexVisual(const HexCoord& coord) const {
    return _spawnedHexes.contains(coord);
}

void World::spawnHexVisual(const std::shared_ptr<HexCell>& cell) {
    if (_spawnedHexes.contains(cell->getCoord())) {
        return;
    }

    auto* scene = _owner->getScene();
    auto* obj = scene->createGameObject(
        std::format("Hex {} {}", cell->getCoord().q(), cell->getCoord().r()), _owner);
    _hexTransforms.insert(obj->transform());
    _spawnedHexes.insert(cell->getCoord());

    auto height = _perlin.noise(static_cast<float>(cell->getCoord().q()) * 0.1F,
                                static_cast<float>(cell->getCoord().r()) * 0.1F) *
                  3.0F;
    std::uniform_real_distribution<float> dist(-0.2F, 0.2F);
    cell->getCoord().setHeight(height + dist(_rng));
    auto worldPos = cell->getCoord().toWorldPosition(1.0F, 0.1F);
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
    if (cell->getType() == HexCell::Type::Bridge) {
        meshRenderer->setColor(glm::vec4(0.0F, 1.0F, 0.0F, 1.0F));
    }
    if (cell->getType() == HexCell::Type::EnemyBattleHex) {
        meshRenderer->setColor(glm::vec4(1.0F, 0.0F, 0.0F, 1.0F));
    }
    meshRenderer->setTransform(obj->transform());
    auto* worldHex = obj->addComponent<WorldHex>();
    worldHex->setHexCell(cell);

    auto* collider = obj->addComponent<dzemikk::Collider>();
    collider->setModel(_model);
    collider->setTransform(obj->transform());
}

void World::setPlayer(PlayerEntity* playerEntity) {
    if (playerEntity == nullptr) {
        return;
    }

    _player = playerEntity;
}
} // namespace game
