#include "scripts/world/worldVisualManager.h"
#include "game.h"

#include <assetManager/assetmanager.h>
#include <ecs/serialize/prefabSerializer.h>
#include <ecs/scene.h>
#include <ecs/components/camera.h>
#include <ecs/components/meshRenderer.h>

#include <glm/glm.hpp>

void game::WorldVisualManager::start() {

}

void game::WorldVisualManager::init() {
    spdlog::warn("[WorldVisualManager] start() called");

    if (_world == nullptr) {
        spdlog::error("[WorldVisualManager] World is nullptr");
        return;
    }

    auto chunks = _world->getVisualHexesByChunk();

    for (const auto& [chunkId, hexes] : chunks) {

        spdlog::info("Chunk {} has {} visual hexes", chunkId,
                     hexes.size());
    }

    _cache["Tree_big"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Tree_big.prefab");
    _cache["Tree_big_with_branches"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Tree_big_with_branches.prefab");
    _cache["Tree_small_with_branches"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Tree_small_with_branches.prefab");
    _cache["Bush"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Bush.prefab");
    _cache["Bush2"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Bush2.prefab");
    _cache["Camp_fire"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Camp_fire.prefab");
    _cache["Rock1"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Rock1.prefab");
    _cache["Rock2"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Rock2.prefab");
    _cache["Rock3"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Rock3.prefab");
    _cache["Tipi"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Tipi.prefab");
    _cache["Tipi_sticks"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Tipi_sticks.prefab");
    _cache["Tree_bush"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Tree_bush.prefab");
    _cache["Tree_smal"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Tree_smal.prefab");
    _cache["Grass1"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Grass1.prefab");
}

void game::WorldVisualManager::spawnPrefabOnChunk(const std::string& chunkName,
                                                  const std::string& prefabKey) {
    if (!_world) {
        spdlog::error("[WorldVisualManager] World is nullptr");
        return;
    }

    auto prefabIt = _cache.find(prefabKey);
    if (prefabIt == _cache.end()) {
        spdlog::error("[WorldVisualManager] Prefab not found: {}", prefabKey);
        return;
    }

    auto chunks = _world->getVisualHexesByChunk();

    auto chunkIt = chunks.find(chunkName);
    if (chunkIt == chunks.end()) {
        spdlog::error("[WorldVisualManager] Chunk not found: {}", chunkName);
        return;
    }

    auto* scene = _world->getOwner()->getScene();

    const auto& hexes = chunkIt->second;

    for (const auto& hex : hexes) {

        if (!hex)
            continue;

        if (hex->getState() == HexCell::State::Prop || hex->getState() == HexCell::State::Item ||
            hex->getState() == HexCell::State::Player || hex->getState() == HexCell::State::Enemy ||
            hex->getState() == HexCell::State::Totem ||
            hex->getState() == HexCell::State::TotemDialog) {
            continue;
        }

        if (hex->getType() == HexCell::Type::PlayerBattleHex ||
            hex->getType() == HexCell::Type::EnemyBattleHex ||
            hex->getType() == HexCell::Type::Bridge ||
            hex->getType() == HexCell::Type::BlockingPattern ||
            hex->getType() == HexCell::Type::BlockingBridge) {
            continue;
        }

        if (hex->getGenState() == HexCell::GenState::Blocked ||
            hex->getGenState() == HexCell::GenState::Protected) {
            continue;
        }

        auto* transform = _world->getHexTransformByCell(*hex);
        if (!transform)
            continue;

        auto* go = dzemikk::PrefabSerializer::instantiate(*scene, *prefabIt->second.get(),
                                                          _assetManager);
        if (!go)
            continue;

        go->transform()->setPosition(transform->getPosition());

        go->setName("ChunkSpawn_" + chunkName);        
        hex->setState(HexCell::State::Prop);
    }

    spdlog::info("[WorldVisualManager] Spawned '{}' on chunk '{}' ({} hexes)", prefabKey, chunkName,
                 hexes.size());
}

void game::WorldVisualManager::spawnForestChunk(const std::string& chunkName) {
    if (!_world)
        return;

    auto chunks = _world->getVisualHexesByChunk();
    auto chunkIt = chunks.find(chunkName);
    if (chunkIt == chunks.end())
        return;

    auto* scene = _world->getOwner()->getScene();

    struct SpawnPoint {
        HexCell* hex;
        glm::vec3 pos;
    };

    const auto& hexes = chunkIt->second;

    std::vector<HexCell*> freeHexes;
    freeHexes.reserve(hexes.size());

    for (auto hex : hexes) {
        if (!hex)
            continue;

        if (!isHexFree(hex.get()))
            continue;

        freeHexes.push_back(hex.get());
    }

    if (freeHexes.empty())
        return;

    std::vector<SpawnPoint> points;
    points.reserve(freeHexes.size());

    glm::vec3 center(0.f);
    int count = 0;

    for (auto* hex : freeHexes) {

        auto* t = _world->getHexTransformByCell(*hex);
        if (!t)
            continue;

        glm::vec3 p = t->getPosition();

        points.push_back({hex, p});

        center += p;
        count++;
    }

    if (count == 0)
        return;

    center /= (float)count;

    glm::vec3 axis(0.f);

    for (auto& point : points) {
        glm::vec3 d = point.pos - center;
        axis += glm::abs(d);
    }

    axis = glm::normalize(axis);

    glm::vec3 right = glm::normalize(glm::cross(axis, glm::vec3(0.f, 1.f, 0.f)));

    auto sideValue = [&](const glm::vec3& p) { return glm::dot(p - center, right); };

    std::vector<SpawnPoint> leftSide;
    std::vector<SpawnPoint> rightSide;

    for (auto& point : points) {
        if (sideValue(point.pos) > 0.f)
            rightSide.push_back(point);
        else
            leftSide.push_back(point);
    }

    int spawned = 0;

    for (auto& point : leftSide) {

        auto* hex = point.hex;
        const glm::vec3& pos = point.pos;

        float fill = 0.65f;
        float r = rand01();

        if (r < fill * 0.45f) {

            float treeRoll = rand01();

            if (treeRoll < 0.30f) {
                spawnClusterObject(scene, "Tree_big_with_branches", pos, 1.0f, 1.4f, _world->getHexTransformByCell(*hex));
            } else if (treeRoll < 0.50f) {
                spawnClusterObject(scene, "Tree_big", pos, 0.8f, 1.2f,
                                   _world->getHexTransformByCell(*hex));
            } else if (treeRoll < 0.65f) {
                spawnClusterObject(scene, "Tree_small_with_branches", pos, 0.7f, 1.1f,
                                   _world->getHexTransformByCell(*hex));
            } else {
                spawnClusterObject(scene, "Tree_smal", pos, 0.8f, 1.2f,
                                   _world->getHexTransformByCell(*hex));
            }

            hex->setState(HexCell::State::Prop);
            spawned++;
        } else if (r < fill * 0.80f) {

            spawnRockCluster(scene, {"Rock1", "Rock2", "Rock3"}, pos, 2, 6, 1.0f, 0.6f, 3.0f,
                             _world->getHexTransformByCell(*hex));
            spawned++;
        } else if (r < fill * 1.0f) {

            spawnClusterObject(scene, "Bush", pos, 0.6f, 0.9f, _world->getHexTransformByCell(*hex));
            spawned++;
        }
    }

    for (auto& point : rightSide) {

        auto* hex = point.hex;
        const glm::vec3& pos = point.pos;

        float r = rand01();

        if (r < 0.55f) {
            spawnClusterObject(scene, "Bush", pos, 0.6f, 0.9f, _world->getHexTransformByCell(*hex));
            spawned++;
        } else if (r < 0.70f) {

            spawnRockCluster(scene, {"Rock1", "Rock2", "Rock3"}, pos, 2, 6, 1.0f, 0.6f, 4.0f,
                             _world->getHexTransformByCell(*hex));

            spawned++;
        } else if (r < 0.80f) {

            float treeRoll = rand01();

            if (treeRoll < 0.65f) {
                spawnClusterObject(scene, "Tree_small_with_branches", pos, 0.7f, 1.1f,
                                   _world->getHexTransformByCell(*hex));
            } else {
                spawnClusterObject(scene, "Tree_smal", pos, 0.8f, 1.2f,
                                   _world->getHexTransformByCell(*hex));
            }

            hex->setState(HexCell::State::Prop);
            spawned++;
        }
    }

    spdlog::info("[WorldVisualManager] Forest chunk '{}' spawned -> {} objects", chunkName,
                 spawned);
}

void game::WorldVisualManager::generatePathBetweenChunks(const std::string& chunkA,
                                                   const std::string& chunkB) {

    auto chunks = _world->getVisualHexesByChunk();

    auto itA = chunks.find(chunkA);
    auto itB = chunks.find(chunkB);

    if (itA == chunks.end() || itB == chunks.end())
        return;

    HexCell* start = getTopHex(itA->second);
    HexCell* target = getTopHex(itB->second);

    if (!start || !target)
        return;

    generatePath(start->getCoord(), target->getCoord());
}

game::HexCell* game::WorldVisualManager::getTopHex(const std::vector<std::shared_ptr<HexCell>>& hexes) {

    HexCell* result = nullptr;
    float bestZ = FLT_MAX;

    for (auto& hex : hexes) {

        auto* t = _world->getHexTransformByCell(*hex);
        if (!t)
            continue;

        if (t->getPosition().z < bestZ) {
            bestZ = t->getPosition().z;
            result = hex.get();
        }
    }

    return result;
}

void game::WorldVisualManager::generatePath(const HexCoord& start, const HexCoord& end) {
    auto path = findPath(start, end);

    for (const auto& coord : path) {

        auto* hex = _world->getGrid()->getCell(coord).get();

        if (!hex)
            continue;

        hex->setVisualState(HexCell::VisualState::Path);
        hex->setDirty(true);
    }
}

std::vector<game::HexCoord> game::WorldVisualManager::findPath(const HexCoord& start, const HexCoord& goal) {
    std::queue<HexCoord> open;
    std::unordered_map<HexCoord, HexCoord> cameFrom;
    std::unordered_set<HexCoord> visited;

    open.push(start);
    visited.insert(start);

    while (!open.empty()) {

        HexCoord current = open.front();
        open.pop();

        if (current == goal)
            break;

        for (const auto& next : HexCoord::getNeighbors(current)) {

            auto cell = _world->getGrid()->getCell(next);

            if (!cell)
                continue;

            if (visited.contains(next))
                continue;

            if (cell->getGenState() == HexCell::GenState::Blocked)
                continue;

            visited.insert(next);
            cameFrom[next] = current;
            open.push(next);
        }
    }

    std::vector<HexCoord> path;

    if (!cameFrom.contains(goal))
        return path;

    for (HexCoord c = goal; c != start; c = cameFrom[c])
        path.push_back(c);

    path.push_back(start);

    std::reverse(path.begin(), path.end());

    return path;
}

bool game::WorldVisualManager::isHexFree(HexCell* hex) const {
    if (!hex)
        return false;

    if (hex->getState() == HexCell::State::Prop || hex->getState() == HexCell::State::Item ||
        hex->getState() == HexCell::State::Player || hex->getState() == HexCell::State::Enemy ||
        hex->getState() == HexCell::State::Totem || hex->getState() == HexCell::State::TotemDialog ||
        hex->getVisualState() == HexCell::VisualState::Path)
        return false;

    if (hex->getType() == HexCell::Type::PlayerBattleHex ||
        hex->getType() == HexCell::Type::EnemyBattleHex ||
        hex->getType() == HexCell::Type::Bridge ||
        hex->getType() == HexCell::Type::BlockingPattern ||
        hex->getType() == HexCell::Type::BlockingBridge)
        return false;

    if (hex->getGenState() == HexCell::GenState::Blocked ||
        hex->getGenState() == HexCell::GenState::Protected)
        return false;

    for (const auto& neighborCoord : HexCoord::getNeighbors(hex->getCoord())) {

        auto neighbor = _world->getGrid()->getCell(neighborCoord);

        if (!neighbor)
            continue;

        if (neighbor->getType() == HexCell::Type::EnemyBattleHex || 
            neighbor->getType() == HexCell::Type::BlockingBridge ||
            neighbor->getType() == HexCell::Type::BlockingPattern ||
            neighbor->getType() == HexCell::Type::Bridge)
            return false;
    }

    return true;
}

void game::WorldVisualManager::spawnClusterObject(dzemikk::Scene* scene,
                                                  const std::string& prefabKey,
                                                  const glm::vec3& pos, float minScale,
                                                  float maxScale, dzemikk::Transform* parent) {
    auto it = _cache.find(prefabKey);
    if (it == _cache.end())
        return;

    auto* go = dzemikk::PrefabSerializer::instantiate(*scene, *it->second.get(), _assetManager, parent->getOwner());
    if (!go)
        return;

    auto* renderer = go->getComponent<dzemikk::MeshRenderer>();
    renderer->setCullingRadius(60.0F);

    if (prefabKey == "Tree_big_with_branches") {
        auto& children = go->getChildren();

        for (auto go : children) {
            auto* renderer = go->getComponent<dzemikk::MeshRenderer>();
            if (renderer) {
                renderer->setCullingRadius(60.0F);
            }
        }

        float killRatio = 0.4f + rand01() * 0.6f;

        std::vector<int> indices(children.size());
        std::iota(indices.begin(), indices.end(), 0);

        std::shuffle(indices.begin(), indices.end(), std::mt19937(rand()));

        int disableCount = (int)(children.size() * killRatio);

        for (int i = 0; i < disableCount && i < (int)indices.size(); i++) {
            children[indices[i]]->enabled(false);
        }
    }

        if (prefabKey == "Tree_small_with_branches") {
        auto& children = go->getChildren();

        for (auto go : children) {
            auto* renderer = go->getComponent<dzemikk::MeshRenderer>();
            if (renderer) {
                renderer->setCullingRadius(60.0F);
            }
        }
    }

    float scale = minScale + rand01() * (maxScale - minScale);
    go->transform()->setScale(glm::vec3(scale));

    if (prefabKey == "Grass1") {
        return;
    }

    float rotY = rand01() * 360.0f;
    glm::quat rot = glm::angleAxis(glm::radians(0.f), glm::vec3(1, 0, 0)) *
                    glm::angleAxis(glm::radians(0.0F), glm::vec3(0, 1, 0)) *
                    glm::angleAxis(glm::radians(rotY), glm::vec3(0, 0, 1));

    go->transform()->setRotation(rot);
}

void game::WorldVisualManager::spawnRockCluster(dzemikk::Scene* scene, const std::vector<std::string>& rocks,
                      const glm::vec3& basePos, int minCount, int maxCount, float spread, float minScale,
                                                float maxScale, dzemikk::Transform* parent) {

    int count = minCount + rand() % (maxCount - minCount + 1);

    for (int i = 0; i < count; i++) {
        std::string prefab = rocks[rand() % rocks.size()];

        glm::vec3 offset((rand01() - 0.5f) * spread, (rand01() - 0.5f) * spread, 0.0f);

        auto* go =
            dzemikk::PrefabSerializer::instantiate(*scene, *_cache[prefab].get(), _assetManager, parent->getOwner());

        if (!go)
            continue;

        auto* renderer = go->getComponent<dzemikk::MeshRenderer>();
        renderer->setCullingRadius(60.0F);

        glm::vec3 pos = offset;
        go->transform()->setPosition(pos);

        float scale = minScale + rand01() * (maxScale - minScale);
        go->transform()->setScale(glm::vec3(scale));

        float rotY = rand01() * 360.0f;
        glm::quat rot = glm::angleAxis(glm::radians(0.0f), glm::vec3(1, 0, 0)) *
                        glm::angleAxis(glm::radians(0.0F), glm::vec3(0, 1, 0)) *
                        glm::angleAxis(glm::radians(rotY), glm::vec3(0, 0, 1));

        go->transform()->setRotation(rot);
    }
}
