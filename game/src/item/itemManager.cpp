#include "item/itemManager.h"

#include "map/ItemEntity.h"
#include "map/ItemEntityBonusHex.h"
#include "map/ItemEntityHealth.h"
#include "map/ItemEntityRevealHex.h"
#include "map/ItemEntityRevealPatter.h"
#include "scripts/world/world.h"

#include <assetManager/assetmanager.h>
#include <ecs/components/meshRenderer.h>
#include <ecs/gameobject.h>
#include <ecs/scene.h>
#include <ecs/serialize/prefabSerializer.h>

namespace game {
ItemManager::ItemManager(unsigned int seed) : _rng(seed) {}

void ItemManager::setWorld(World* world) {
    _world = world;
}

void ItemManager::setAssetManager(dzemikk::AssetManager* assetManager) {
    _assetManager = assetManager;
}

void ItemManager::setGame(Game* game) {
    _game = game;
}

void ItemManager::addItem(const boost::uuids::uuid& chunkId, ItemSpawnConfig config) {
    if (!_world || !_game || !_assetManager) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[ItemManager] World/Game/AssetManager is/are not set. Cannot add item: {}",
                     static_cast<int>(config.type));
        return;
#endif
    }

    config.chunkId = chunkId;
    _spawnRules[chunkId].emplace_back(std::move(config));

    auto cells = collectAvailableCells(_world->getGrid()->getChunkById(chunkId));

    std::shuffle(cells.begin(), cells.end(), _rng);
    spawnItem(chunkId, cells.front(), config);
}

void ItemManager::addItem(const boost::uuids::uuid& chunkId, ItemSpawnConfig config,
                          const HexCoord& coord) {
    if (!_world || !_game || !_assetManager) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[ItemManager] World/Game/AssetManager is/are not set. Cannot add item: {}",
                     static_cast<int>(config.type));
#endif
        return;
    }

    config.chunkId = chunkId;
    _spawnRules[chunkId].emplace_back(std::move(config));

    auto chunk = _world->getGrid()->getChunks().find(chunkId);
    if (chunk == _world->getGrid()->getChunks().end()) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[ItemManager] Chunk with id {} not found. Cannot add item: {}",
                     boost::uuids::to_string(chunkId), static_cast<int>(config.type));
#endif
        return;
    }

    auto cellPtr = chunk->second->getCell(coord);
    spawnItem(chunkId, cellPtr, config);
}

std::vector<HexChunk::HexCellPtr> ItemManager::collectAvailableCells(HexChunk* chunk) {
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
        if (cell->getEntity() != nullptr) {
            continue;
        }
        result.push_back(cell);
    }

    return result;
}

void ItemManager::spawnItem(const boost::uuids::uuid& chunkId, const HexChunk::HexCellPtr& cell,
                            const ItemSpawnConfig& cfg) {
    if (!cell) {
        return;
    }

    ItemEntity* item = nullptr;
    dzemikk::GameObject* go = nullptr;
    std::string prefabPath;

    switch (cfg.type) {
    case ItemType::Heal: {
        prefabPath = "prefabs/ItemHeal.prefab";
        break;
    }
    case ItemType::RevealPattern: {
        prefabPath = "prefabs/ItemRevealPattern.prefab";
        break;
    }
    case ItemType::RevealHex: {
        prefabPath = "prefabs/ItemRevealHex.prefab";
        break;
    }
    case ItemType::BonusHex: {
        prefabPath = "prefabs/ItemBonusHex.prefab";
        break;
    }
    }

    auto prefab = _assetManager->get<nlohmann::json>(prefabPath);
    go = dzemikk::PrefabSerializer::instantiate(*getOwner()->getScene(), *prefab.get(),
                                                _assetManager, getOwner());

    switch (cfg.type) {
    case ItemType::Heal: {
        float healAmount = cfg.healAmount.value_or(10.0F);
        item = go->addComponent<ItemEntityHealth>(healAmount);
        break;
    }
    case ItemType::RevealPattern: {
        item = go->addComponent<ItemEntityRevealPattern>();
        break;
    }
    case ItemType::RevealHex: {
        item = go->addComponent<ItemEntityRevealHex>();
        break;
    }
    case ItemType::BonusHex: {
        HexPattern pattern =
            cfg.bonusPattern.value_or(HexPattern({{0, 0}}, HexPattern::Type::BONUSHEX));
        item = go->addComponent<ItemEntityBonusHex>(pattern);
        break;
    }
    }

    item->setConfig(cfg);
    // NOTE: Kind of experimental. Theoretically possible and *should not* break anything here. But
    // if smth breaks call the ambulance.
    item->setId(cfg.persistantId);

    if (item != nullptr && go != nullptr) {
        cell->setState(HexCell::State::Item);
        cell->setEntity(item);
        item->onEnter(cell);

        auto worldPos = cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight());
        go->transform()->setPosition(worldPos + glm::vec3(0.0F, 0.5F, 0.0F));

        _spawnedItems[chunkId].push_back(item);
    }
}

void ItemManager::update(double dt) {
    if (!_world) {
        return;
    }

    for (auto& [uuid, itemEntities] : _spawnedItems) {
        for (auto it = itemEntities.begin(); it != itemEntities.end();) {
            auto* item = *it;
            if (item->isConsumed()) {
                auto& rules = _spawnRules[uuid];
                const auto& itemCfg = item->getConfig();
                std::erase_if(rules, [&](const ItemSpawnConfig& cfg) { return cfg == itemCfg; });

                item->getCell()->setEntity(nullptr);
                item->getCell()->setState(HexCell::State::Empty);
                auto* owner = item->getOwner();
                it = itemEntities.erase(it);
                owner->destroy();
            } else {
                ++it;
            }
        }
    }
}

nlohmann::json ItemManager::saveState() const {
    nlohmann::json j;
    for (const auto& [chunkId, items] : _spawnedItems) {
        for (const auto* item : items) {
            j["items"][boost::uuids::to_string(item->getConfig().persistantId)] = item->getConfig();
            j["items"][boost::uuids::to_string(item->getConfig().persistantId)]["gridPos"] =
                item->getCell()->getCoord();
        }
    };

    return j;
}

void ItemManager::loadState(const nlohmann::json& j) {
    for (const auto& [idStr, itemData] : j.items()) {
        auto id = boost::uuids::string_generator()(idStr);
        ItemSpawnConfig cfg = itemData.get<ItemSpawnConfig>();
        HexCoord coord = itemData["gridPos"].get<HexCoord>();
        addItem(cfg.chunkId, cfg, coord);
    }
}

} // namespace game
