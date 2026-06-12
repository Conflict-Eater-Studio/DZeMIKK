#include "enemySystem/enemyManager.h"

#include "enemySystem/enemyEntity.h"
#include "enemySystem/territoryPatternRegistry.h"
#include "scripts/world/world.h"

#include <animation/animationstatemachine.h>
#include <assetManager/assetmanager.h>
#include <ecs/components/animator.h>
#include <ecs/components/skinnedMeshRenderer.h>
#include <ecs/gameobject.h>
#include <ecs/scene.h>
#include <ecs/serialize/prefabSerializer.h>
#include <iostream>

game::EnemyManager::EnemyManager(unsigned int seed) : _rng(seed) {}

void game::EnemyManager::setWorld(World* world) {
    _world = world;
}

void game::EnemyManager::setAssetManager(dzemikk::AssetManager* assetManager) {
    _assetManager = assetManager;
}

void game::EnemyManager::setSpawnConfig(const boost::uuids::uuid& chunkId,
                                        const std::vector<EnemySpawnConfig>& config) {
    _spawnRules[chunkId] = config;
}

void game::EnemyManager::spawnEnemiesPerChunk() {
    if (!_world || !_assetManager) {
        return;
    }

    const auto& chunks = _world->getGrid()->getChunks();

    for (const auto& [chunkId, chunkPtr] : chunks) {

        auto ruleIt = _spawnRules.find(chunkId);

        if (ruleIt == _spawnRules.end()) {
            continue;
        }

        auto availableCells = collectAvailableCells(chunkPtr.get());

        if (availableCells.empty()) {
            continue;
        }

        std::shuffle(availableCells.begin(), availableCells.end(), _rng);

        size_t cursor = 0;

        for (const auto& cfg : ruleIt->second) {
            spawnFromConfig(cfg, availableCells, cursor, chunkId);
        }
    }
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

void game::EnemyManager::spawnFromConfig(const EnemySpawnConfig& cfg,
                                         std::vector<HexChunk::HexCellPtr>& availableCells,
                                         size_t& cursor, const boost::uuids::uuid& spawnChunkId) {

    const auto* pattern = TerritoryPatternRegistry::instance().get(cfg.territoryPattern);

    if (!pattern) {
        return;
    }

    for (int i = 0; i < cfg.count; ++i) {

        while (cursor < availableCells.size()) {

            const game::HexGrid::HexCellPtr cell = availableCells[cursor++];

            if (!canPlacePattern(cell->getCoord(), *pattern)) {
                continue;
            }

            spawnEnemy(cell, cfg, spawnChunkId);
            break;
        }
    }
}

void game::EnemyManager::spawnEnemy(HexChunk::HexCellPtr cell, const EnemySpawnConfig& cfg,
                                    const boost::uuids::uuid& spawnChunkId) {
    if (!_assetManager || !cell) {
        return;
    }

    std::string prefabPath;

    switch (cfg.personality) {
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

    enemy->setHp(cfg.hp);
    enemy->setEnemyType(cfg.type);
    enemy->setEnemyPersonality(cfg.personality);
    enemy->setConfig(cfg);

    enemy->onEnter(cell);

    cell->setEntity(enemy);
    cell->setState(HexCell::State::Enemy);

    const auto* pattern = TerritoryPatternRegistry::instance().get(cfg.territoryPattern);

    if (pattern) {
        assignTerritory(enemy, cell, *pattern);
    }

    for (const auto& blockedChunkId : cfg.blocksChunks) {
        _world->getGrid()->lockBridge({spawnChunkId, blockedChunkId}, enemy->getId());
    }

    auto chunkDefIt =
        std::ranges::find_if(_world->getWorldDefinition().chunks, [&](const auto& chunkDef) {
            return chunkDef.chunkId == spawnChunkId;
        });
    chunkDefIt->enemies.emplace_back(enemy);
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

        if (!cell) {
            return false;
        }

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
