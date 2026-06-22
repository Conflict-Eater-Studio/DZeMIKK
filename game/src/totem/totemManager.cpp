#include "totem/totemManager.h"

#include "game.h"
#include "scripts/world/world.h"
#include "totem/totemEntity.h"

#include <assetManager/assetmanager.h>
#include <ecs/components/meshRenderer.h>
#include <ecs/gameobject.h>
#include <ecs/serialize/prefabSerializer.h>
#include <renderer/texture.h>
#include <unordered_map>

game::TotemManager::TotemManager(unsigned int seed) : _rng(seed) {}

void game::TotemManager::setWorld(World* world) {
    _world = world;
}

void game::TotemManager::setAssetManager(dzemikk::AssetManager* assetManager) {
    _assetManager = assetManager;
}

void game::TotemManager::setGame(Game* game) {
    _game = game;
}

void game::TotemManager::addTotem(const boost::uuids::uuid& chunkId, TotemSpawnConfig config) {
    if (!_world || !_game || !_assetManager) {
        return;
    }

    config.chunkId = chunkId;
    _spawnRules[chunkId].emplace_back(config);

    auto cells = collectAvailableCells(_world->getGrid()->getChunkById(chunkId));

    if (cells.empty()) {
        return;
    }

    std::shuffle(cells.begin(), cells.end(), _rng);
    spawnTotem(chunkId, cells.front(), config);
}

void game::TotemManager::addTotem(const boost::uuids::uuid& chunkId, TotemSpawnConfig config,
                                  const HexCoord& coord) {
    if (!_world || !_game || !_assetManager) {
        return;
    }

    config.chunkId = chunkId;
    _spawnRules[chunkId].emplace_back(config);

    auto chunk = _world->getGrid()->getChunks().find(chunkId);
    if (chunk == _world->getGrid()->getChunks().end()) {
        return;
    }

    auto cellPtr = chunk->second->getCell(coord);
    spawnTotem(chunkId, cellPtr, config);
}

std::vector<game::HexChunk::HexCellPtr> game::TotemManager::collectAvailableCells(HexChunk* chunk) {
    std::vector<HexChunk::HexCellPtr> result;

    for (auto& [coord, cell] : chunk->getHexes()) {

        if (!cell) {
            continue;
        }

        if (cell->getState() != HexCell::State::Empty) {
            continue;
        }

        if (cell->getGenState() != HexCell::GenState::Normal) {
            continue;
        }

        if (cell->getType() != HexCell::Type::Normal) {
            continue;
        }

        result.push_back(cell);
    }

    return result;
}

void game::TotemManager::spawnTotem(const boost::uuids::uuid& chunkId,
                                    const HexChunk::HexCellPtr& cell, const TotemSpawnConfig& cfg) {
    if (!cell) {
        return;
    }

    auto prefab = _assetManager->get<nlohmann::json>(cfg.prefabPath);

    auto* scene = getOwner()->getScene();

    auto* totemGO = dzemikk::PrefabSerializer::instantiate(*scene, *prefab.get(), _assetManager,
                                                           this->getOwner());

    totemGO->addTag("Totem");

    totemGO->setParent(getOwner());

    totemGO->transform()->setPosition(
        cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
        glm::vec3(0.F, 0.9F, 0.F));

    totemGO->transform()->setScale({0.8, 0.8, 0.8});

    auto* rendererTotemGO =
        totemGO->findChildByName("platform")->getComponent<dzemikk::MeshRenderer>();
    rendererTotemGO->setCullingRadius(60.0F);

    rendererTotemGO = totemGO->findDescendantByName("Left")->getComponent<dzemikk::MeshRenderer>();
    rendererTotemGO->setCullingRadius(60.0F);

    rendererTotemGO = totemGO->findDescendantByName("Right")->getComponent<dzemikk::MeshRenderer>();
    rendererTotemGO->setCullingRadius(60.0F);

    auto* totem = totemGO->addComponent<TotemEntity>();

    totem->setConfig(cfg);
    totem->setId(cfg.persistantId);
    totem->setGame(_game);

    totem->onEnter(cell);

    _spawnedTotems[chunkId].push_back(totem);

    std::uniform_int_distribution<size_t> dist(0, cfg.segmentPool.size() - 1);

    for (int i = 0; i < 3; i++) {

        const auto& prefabPath = cfg.segmentPool[dist(_rng)];

        auto segmentPrefab = _assetManager->get<nlohmann::json>(prefabPath);

        auto* segmentGO =
            dzemikk::PrefabSerializer::instantiate(*scene, *segmentPrefab.get(), _assetManager,
                                                   this->getOwner()->findDescendantByName("Totem"));

        segmentGO->setParent(totemGO);

        segmentGO->transform()->setPosition(glm::vec3(0.0F, static_cast<float>(i), 0.0F));

        auto* rendererSegmentGO = segmentGO->getComponent<dzemikk::MeshRenderer>();
        rendererSegmentGO->setCullingRadius(60.0F);

        auto texture = _assetManager->get<dzemikk::Texture>("textures/totems/a.png");

        rendererSegmentGO->getMaterial(0)->setAlbedoTexture(texture);
    }

    if (cfg.used) {
        totem->lightOff();
    }
}

nlohmann::json game::TotemManager::saveState() const {
    nlohmann::json j;
    for (const auto& [chunkId, totems] : _spawnedTotems) {
        for (const auto* totem : totems) {
            j["totems"][boost::uuids::to_string(totem->getConfig().persistantId)] =
                totem->getConfig();
            j["totems"][boost::uuids::to_string(totem->getConfig().persistantId)]["gridPos"] =
                totem->getCell()->getCoord();
        }
    };

    return j;
}

void game::TotemManager::loadState(const nlohmann::json& j) {
    for (const auto& [idStr, totemData] : j.items()) {
        TotemSpawnConfig cfg = totemData.get<TotemSpawnConfig>();
        HexCoord coord = totemData["gridPos"].get<HexCoord>();
        addTotem(cfg.chunkId, cfg, coord);
    }
}

void game::TotemManager::clear() {
    for (auto& [chunkId, totems] : _spawnedTotems) {
        for (auto* totem : totems) {
            if (auto cell = totem->getCell(); cell) {
                cell->setEntity(nullptr);
                cell->setState(HexCell::State::Empty);
            }
            if (auto* owner = totem->getOwner(); owner) {
                owner->destroy();
            }
        }
    }
    _spawnedTotems.clear();
    _spawnRules.clear();
}

void game::TotemManager::markTotemUsed(const boost::uuids::uuid& persistantId) {
    for (auto& [chunkId, totems] : _spawnedTotems) {
        for (auto* totem : totems) {
            if (totem->getConfig().persistantId == persistantId) {
                if (!totem->getConfig().used) {
                    totem->lightOff();
                }
                return;
            }
        }
    }
}

void game::TotemManager::markTotemUnused(const boost::uuids::uuid& persistantId) {
    for (auto& [chunkId, totems] : _spawnedTotems) {
        for (auto* totem : totems) {
            if (totem->getConfig().persistantId == persistantId) {
                if (totem->getConfig().used) {
                    totem->unuse();
                }
                return;
            }
        }
    }
}
