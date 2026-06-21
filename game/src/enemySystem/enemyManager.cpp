#include "enemySystem/enemyManager.h"

#include "enemySystem/enemyEntity.h"
#include "enemySystem/enemyTypes.h"
#include "enemySystem/territoryPatternRegistry.h"
#include "scripts/world/world.h"

#include <animation/animationstatemachine.h>
#include <assetManager/assetmanager.h>
#include <ecs/components/animator.h>
#include <ecs/components/skinnedMeshRenderer.h>
#include <ecs/gameobject.h>
#include <ecs/scene.h>
#include <ecs/serialize/prefabSerializer.h>

game::EnemyManager::EnemyManager(unsigned int seed) : _rng(seed) {}

void game::EnemyManager::setWorld(World* world) {
    _world = world;
}

void game::EnemyManager::setAssetManager(dzemikk::AssetManager* assetManager) {
    _assetManager = assetManager;
}

void game::EnemyManager::addEnemy(const boost::uuids::uuid& chunkId, EnemySpawnConfig config) {
    if (!_world || !_assetManager) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[ItemManager] World/AssetManager is/are not set. Cannot add enemy");
        return;
#endif
    }

    config.chunkId = chunkId;
    _spawnRules[chunkId].emplace_back(config);

    auto cells = collectAvailableCells(_world->getGrid()->getChunkById(chunkId));

    if (cells.empty()) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[EnemyManager] No available cells in chunk. Enemy won't be spawned");
#endif
        return;
    }

    std::shuffle(cells.begin(), cells.end(), _rng);

    for (auto& cell : cells) {
        auto* pattern = TerritoryPatternRegistry::instance().get(config.territoryPattern);
        if (pattern && !canPlacePattern(cell->getCoord(), *pattern)) {
            continue;
        }
        spawnEnemy(chunkId, cell, config);
        return;
    }

#if DZEMIKK_DEV_TOOLS
    spdlog::warn(
        "[EnemyManager] Can't place territory pattern on any cell. Enemy won't be spawned");
#endif
}

void game::EnemyManager::addEnemy(const boost::uuids::uuid& chunkId, EnemySpawnConfig config,
                                  const HexCoord& coord) {
    if (!_world || !_assetManager) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[ItemManager] World/AssetManager is/are not set. Cannot add enemy");
        return;
#endif
    }

    config.chunkId = chunkId;
    _spawnRules[chunkId].emplace_back(config);

    auto chunk = _world->getGrid()->getChunks().find(chunkId);
    if (chunk == _world->getGrid()->getChunks().end()) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[EnemyManager] Chunk with id {} not found. Cannot add enemy",
                     boost::uuids::to_string(chunkId));
#endif
        return;
    }

    auto cellPtr = chunk->second->getCell(coord);
    spawnEnemy(chunkId, cellPtr, config, true);
}

std::vector<game::HexChunk::HexCellPtr> game::EnemyManager::collectAvailableCells(HexChunk* chunk) {
    std::vector<HexChunk::HexCellPtr> result;

    for (const auto& [coord, cell] : chunk->getHexes()) {
        if (cell && cell->getState() == HexCell::State::Empty &&
            cell->getGenState() == HexCell::GenState::Normal &&
            cell->getType() == HexCell::Type::Normal) {
            result.push_back(cell);
        }
    }

    return result;
}

void game::EnemyManager::spawnEnemy(const boost::uuids::uuid& chunkId,
                                    const HexChunk::HexCellPtr& cell,
                                    const EnemySpawnConfig& config, bool skipValidation) {
    if (!_assetManager || !cell) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[EnemyManager] Asset Manager not set. Enemy won't be spawned");
#endif
        return;
    }

    const auto* pattern = TerritoryPatternRegistry::instance().get(config.territoryPattern);

    if (!pattern) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[EnemyManager] Territory pattern not found: {}. Enemy won't be spawned",
                     config.territoryPattern);
#endif
        return;
    }

    if (!skipValidation && !canPlacePattern(cell->getCoord(), *pattern)) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[EnemyManager] Can't place territory pattern. Enemy won't be spawned");
#endif
        return;
    }

    std::string prefabPath;

    switch (config.personality) {
    case EnemyPersonality::Aggressive:
        prefabPath = "prefabs/aggressive_enemy.prefab";
        break;

    case EnemyPersonality::Defensive:
        prefabPath = "prefabs/defensive_enemy.prefab";
        break;

    case EnemyPersonality::Balanced:
        prefabPath = "prefabs/balanced_enemy.prefab";
        break;
    }

    auto enemyPrefab = _assetManager->get<nlohmann::json>(prefabPath);

    auto* scene = getOwner()->getScene();
    auto* enemyGO =
        dzemikk::PrefabSerializer::instantiate(*scene, *enemyPrefab.get(), _assetManager);

    enemyGO->addTag("Enemy");
    enemyGO->setParent(getOwner());

    enemyGO->transform()->setPosition(
        cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
        glm::vec3(0.0F, 0.4F, 0.0F));

    dzemikk::AnimationClip* clip = nullptr;
    auto skeleton =
        enemyGO->getComponent<dzemikk::SkinnedMeshRenderer>()->getModel().get()->getSkeleton();
    clip = skeleton->getClip("mixamo.com");
    auto* animator = enemyGO->getComponent<dzemikk::Animator>();
    animator->getStateMachine()->getState("Idle")->setClip(clip);
    animator->play("Idle");

    auto* enemy = enemyGO->addComponent<EnemyEntity>();

    enemy->setId(config.persistantId);
    enemy->setHp(config.hp);
    enemy->setEnemyType(config.type);
    enemy->setEnemyPersonality(config.personality);
    enemy->setConfig(config);

    enemy->onEnter(cell);

    cell->setEntity(enemy);
    cell->setState(HexCell::State::Enemy);

    _spawnedEnemies[chunkId].emplace_back(enemy);

    if (pattern) {
        assignTerritory(enemy, cell, *pattern);
    }

    for (const auto& blockedChunkId : config.blocksChunks) {
        _world->getGrid()->lockBridge({chunkId, blockedChunkId}, enemy->getId());
    }
}

void game::EnemyManager::assignTerritory(EnemyEntity* enemy, HexChunk::HexCellPtr centerCell,
                                         const TerritoryPattern& pattern) {
    auto* grid = _world->getGrid();
    const HexCoord center = centerCell->getCoord();
    HexChunk* chunk = grid->findChunkForCoord(center);

    for (const auto& offset : pattern.offsets) {
        HexCoord coord = center + offset;

        auto targetCell = grid->getCell(coord);
        bool hasVisual = _world->hasHexVisual(coord);

        if (!targetCell) {
            auto newCell =
                std::make_shared<HexCell>(coord, HexCell::State::Empty,
                                          HexCell::Type::EnemyBattleHex, HexCell::GenState::Normal);

            if (!chunk) {
                chunk = _world->getGrid()->getChunks().begin()->second.get();
            }

            chunk->insertCell(coord, newCell);

            targetCell = newCell;
        }

        if (!hasVisual) {
            _world->ensureHexExists(targetCell);
        }

        enemy->addTerritoryCell(targetCell.get());
        _cellToEnemy[targetCell->getCoord()] = enemy;

        targetCell->setType(HexCell::Type::EnemyBattleHex);
        _world->reserveTerritory(targetCell->getCoord());
        targetCell->setDirty(true);
    }
}

game::HexChunk* game::EnemyManager::findChunkForCoord(const game::HexCoord& coord) {
    const auto& chunks = _world->getGrid()->getChunks();

    for (const auto& [id, chunkPtr] : chunks) {
        if (chunkPtr->contains(coord)) {
            return chunkPtr.get();
        }
    }
    return nullptr;
}

bool game::EnemyManager::canPlacePattern(HexCoord center, const game::TerritoryPattern& pattern) {
    auto* grid = _world->getGrid();

    for (const auto& offset : pattern.offsets) {
        HexCoord c = center + offset;

        if (_world->isTerritoryReserved(c)) {
            return false;
        }

        auto cell = grid->getCell(c);

        if (cell) {
            if (cell->getState() != HexCell::State::Empty) {
                return false;
            }

            if (cell->getGenState() != HexCell::GenState::Normal) {
                return false;
            }

            if (cell->getType() != HexCell::Type::Normal) {
                return false;
            }
        }
    }

    return true;
}

game::EnemyEntity* game::EnemyManager::getEnemyByCell(game::HexCell* cell) const {
    if (!cell) {
        return nullptr;
    }

    auto it = _cellToEnemy.find(cell->getCoord());
    if (it != _cellToEnemy.end()) {
        return it->second;
    }

    return nullptr;
}

nlohmann::json game::EnemyManager::saveState() const {
    nlohmann::json j;
    for (const auto& [chunkId, enemies] : _spawnedEnemies) {
        for (const auto* enemy : enemies) {
            auto key = boost::uuids::to_string(enemy->getConfig().persistantId);
            j["enemies"][key] = enemy->getConfig();
            j["enemies"][key]["gridPos"] = enemy->getCell()->getCoord();
            j["enemies"][key]["chunkId"] = boost::uuids::to_string(chunkId);
        }
    };

    return j;
}

void game::EnemyManager::loadState(const nlohmann::json& j) {
    for (const auto& [idStr, itemData] : j.items()) {
        game::EnemySpawnConfig cfg = itemData.get<game::EnemySpawnConfig>();
        HexCoord coord = itemData["gridPos"].get<HexCoord>();
        auto chunkId = boost::uuids::string_generator()(itemData.at("chunkId").get<std::string>());

        cfg.chunkId = chunkId;
        _spawnRules[chunkId].emplace_back(cfg);

        auto cellPtr = _world->getGrid()->getChunkById(chunkId)->getCell(coord);
        spawnEnemy(chunkId, cellPtr, cfg, true);
    }
}

void game::EnemyManager::clear() {
    for (auto& [chunkId, enemies] : _spawnedEnemies) {
        for (auto* enemy : enemies) {
            if (auto cell = enemy->getCell(); cell) {
                cell->setEntity(nullptr);
                cell->setState(HexCell::State::Empty);
            }
            if (auto* owner = enemy->getOwner(); owner) {
                owner->destroy();
            }
        }
    }
    _spawnedEnemies.clear();
    _spawnRules.clear();
    _cellToEnemy.clear();
    _removedEnemyIds.clear();
}

std::vector<boost::uuids::uuid> game::EnemyManager::getAndClearRemovedEnemyIds() {
    auto result = std::move(_removedEnemyIds);
    _removedEnemyIds.clear();
    return result;
}

void game::EnemyManager::removeEnemy(game::EnemyEntity* enemy) {
    if (!enemy) {
        return;
    }

    _removedEnemyIds.push_back(enemy->getConfig().persistantId);

    auto chunkId = enemy->getConfig().chunkId;

    _spawnRules[chunkId].erase(
        std::remove_if(_spawnRules[chunkId].begin(), _spawnRules[chunkId].end(),
                       [&](const EnemySpawnConfig& cfg) {
                           return cfg.persistantId == enemy->getConfig().persistantId;
                       }),
        _spawnRules[chunkId].end());
    if (_spawnRules[chunkId].empty()) {
        _spawnRules.erase(chunkId);
    }

    _spawnedEnemies[chunkId].erase(
        std::remove_if(_spawnedEnemies[chunkId].begin(), _spawnedEnemies[chunkId].end(),
                       [&](const EnemyEntity* e) { return e->getId() == enemy->getId(); }),
        _spawnedEnemies[chunkId].end());
    if (_spawnedEnemies[chunkId].empty()) {
        _spawnedEnemies.erase(chunkId);
    }
}
