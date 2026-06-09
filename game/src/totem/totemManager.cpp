#include "totem/totemManager.h"
#include "scripts/world/world.h"
#include "totem/totemEntity.h"

#include <assetManager/assetmanager.h>
#include <ecs/gameobject.h>
#include <ecs/serialize/prefabSerializer.h>

game::TotemManager::TotemManager(unsigned int seed) : _rng(seed) {}

void game::TotemManager::setWorld(World* world) {
    _world = world;
}

void game::TotemManager::setAssetManager(dzemikk::AssetManager* assetManager) {
    _assetManager = assetManager;
}

void game::TotemManager::setSpawnConfig(const boost::uuids::uuid& chunkId,
                                  const std::vector<TotemSpawnConfig>& config) {
    _spawnRules[chunkId] = config;
}

std::vector<game::HexChunk::HexCellPtr> game::TotemManager::collectAvailableCells(HexChunk* chunk) {
    std::vector<HexChunk::HexCellPtr> result;

    for (auto& [coord, cell] : chunk->getHexes()) {

        if (!cell)
            continue;

        if (cell->getState() != HexCell::State::Empty)
            continue;

        if (cell->getGenState() != HexCell::GenState::Normal)
            continue;

        result.push_back(cell);
    }

    return result;
}

void game::TotemManager::spawnTotemsPerChunk() {
    if (!_world || !_assetManager)
        return;

    const auto& chunks = _world->getGrid()->getChunks();

    for (const auto& [chunkId, chunk] : chunks) {
        auto it = _spawnRules.find(chunkId);

        if (it == _spawnRules.end())
            continue;

        auto available = collectAvailableCells(chunk.get());

        std::shuffle(available.begin(), available.end(), _rng);

        size_t cursor = 0;

        for (const auto& cfg : it->second) {
            for (int i = 0; i < cfg.count && cursor < available.size(); i++) {
                spawnTotem(available[cursor++], cfg);
            }
        }
    }
}

void game::TotemManager::spawnTotem(HexChunk::HexCellPtr cell, const TotemSpawnConfig& cfg) {
    if (!cell)
        return;

    auto prefab = _assetManager->get<nlohmann::json>(cfg.prefabPath);

    auto* scene = getOwner()->getScene();

    auto* totemGO = dzemikk::PrefabSerializer::instantiate(*scene, *prefab.get(), _assetManager, this->getOwner());

    totemGO->addTag("Totem");

    totemGO->setParent(getOwner());

    totemGO->transform()->setPosition(
        cell->getCoord().toWorldPosition(1.0f, 0.1f, cell->getHeight()) +
        glm::vec3(0.f, 1.25f, 0.f));

    totemGO->transform()->setScale({0.8, 0.8, 0.8});

    auto* totem = totemGO->addComponent<TotemEntity>();

    totem->setConfig(cfg);

    totem->onEnter(cell);

        std::uniform_int_distribution<size_t> dist(0, cfg.segmentPool.size() - 1);

    for (int i = 0; i < 3; i++) {

        const auto& prefabPath = cfg.segmentPool[dist(_rng)];

        auto segmentPrefab = _assetManager->get<nlohmann::json>(prefabPath);

        auto* segmentGO =
            dzemikk::PrefabSerializer::instantiate(*scene, *segmentPrefab.get(), _assetManager, this->getOwner()->findDescendantByName("Totem"));

        segmentGO->setParent(totemGO);

        segmentGO->transform()->setPosition(glm::vec3(0.0f, static_cast<float>(i), 0.0f));
    }
}