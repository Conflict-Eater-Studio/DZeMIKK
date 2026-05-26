#include "enemySystem/enemyManager.h"
#include "enemySystem/enemyEntity.h"
#include "scripts/world/world.h"

#include <ecs/gameobject.h>
#include <ecs/scene.h>
#include <ecs/serialize/prefabSerializer.h>
#include <assetManager/assetmanager.h>
#include <ecs/components/skinnedMeshRenderer.h>
#include <ecs/components/animator.h>1
#include <animation/animationstatemachine.h>
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
    if (!_world || !_assetManager)
        return;

    auto& chunks = _world->getGrid()->getChunks();

    for (const auto& [chunkId, chunkPtr] : chunks) {
        auto ruleIt = _spawnRules.find(chunkId);
        if (ruleIt == _spawnRules.end())
            continue;

        HexChunk* chunk = chunkPtr.get();
        const auto& configs = ruleIt->second;

        std::vector<HexChunk::HexCellPtr> availableCells;
        for (const auto& [coord, cell] : chunk->getHexes()) {
            if (cell && cell->getState() == HexCell::State::Empty && cell->getGenState() == HexCell::GenState::Normal) {
                availableCells.push_back(cell);
            }
        }

        if (availableCells.empty())
            continue;

        std::shuffle(availableCells.begin(), availableCells.end(), _rng);

        size_t cursor = 0;

        for (const auto& cfg : configs) {

            for (int i = 0; i < cfg.count; i++) {

                if (cursor >= availableCells.size())
                    break;

                auto cell = availableCells[cursor++];
                spawnEnemy(cell, cfg);
            }
        }
    }
}

void game::EnemyManager::spawnEnemy(HexChunk::HexCellPtr cell, const EnemySpawnConfig& cfg) {
    if (!_assetManager || !cell)
        return;
    
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

    enemyGO->setParent(getOwner());

    enemyGO->transform()->setPosition(cell->getCoord().toWorldPosition(1.0F, 0.1F) +
                                      glm::vec3(0.0F, 0.4F, 0.0F));

    dzemikk::AnimationClip* clip = nullptr;
    auto skeleton =
        enemyGO->getComponent<dzemikk::SkinnedMeshRenderer>()->getModel().get()->getSkeleton();
    clip = skeleton->getClip("mixamo.com");
    auto animator = enemyGO->getComponent<dzemikk::Animator>();
    animator->getStateMachine()->getState("Idle")->setClip(clip);
    animator->play("Idle");

    auto* enemy = enemyGO->addComponent<EnemyEntity>();


    //enemy->setHP(cfg.hp);
    //enemy->setType(cfg.type);

    enemy->onEnter(cell);

    cell->setEntity(enemy);
    cell->setState(HexCell::State::Enemy);
}
