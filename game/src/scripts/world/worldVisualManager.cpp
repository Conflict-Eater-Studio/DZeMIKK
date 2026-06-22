#define GLM_ENABLE_EXPERIMENTAL

#include "scripts/world/worldVisualManager.h"
#include "game.h"

#include <assetManager/assetmanager.h>
#include <ecs/serialize/prefabSerializer.h>
#include <ecs/scene.h>
#include <ecs/components/camera.h>
#include <ecs/components/meshRenderer.h>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

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
    _cache["Signpost"] = _assetManager->get<nlohmann::json>("prefabs/map_assest/Signpost.prefab");
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
                                                         const std::string& chunkB,
                                                         HexCell::Type chunkAtype,
                                                         HexCell::Type chunkBtype) {

    auto chunks = _world->getVisualHexesByChunk();

    auto itA = chunks.find(chunkA);
    auto itB = chunks.find(chunkB);

    if (itB == chunks.end()) {
        auto itB = chunks.find(chunkA);
    }

    if (itA == chunks.end() || itB == chunks.end())
        return;

    HexCell* start = getTopHex(itA->second);
    HexCell* target = getTopHex(itB->second);

    if (chunkAtype == HexCell::Type::BlockingBridge || chunkAtype == HexCell::Type::BlockingPattern || chunkAtype == HexCell::Type::Bridge) {
        start = getExtremeHexOfType(itA->second, chunkAtype, true);
    }

    if (chunkBtype == HexCell::Type::BlockingBridge ||
        chunkBtype == HexCell::Type::BlockingPattern || chunkBtype == HexCell::Type::Bridge || chunkBtype == HexCell::Type::EnemyBattleHex) {
        target = getExtremeHexOfType(itB->second, chunkBtype, true);
    }

    if (!start || !target)
        return;

    generatePath(start->getCoord(), target->getCoord());
}

void game::WorldVisualManager::spawnSignToChunk(const std::string& sourceChunk,
                                                const std::string& targetChunk) {
    if (!_world)
        return;

    auto chunks = _world->getVisualHexesByChunk();

    auto sourceIt = chunks.find(sourceChunk);
    auto targetIt = chunks.find(targetChunk);

    if (sourceIt == chunks.end() || targetIt == chunks.end())
        return;

    HexCell* bridgeHex = nullptr;

    for (auto& hexPtr : targetIt->second) {
        if (!hexPtr)
            continue;

        if (hexPtr->getType() == HexCell::Type::Bridge ||
            hexPtr->getType() == HexCell::Type::BlockingBridge) {
            bridgeHex = hexPtr.get();
            break;
        }
    }

    if (!bridgeHex)
        return;

    auto* bridgeTransform = _world->getHexTransformByCell(*bridgeHex);

    if (!bridgeTransform)
        return;

    std::vector<HexCell*> candidates;

    for (auto& hexPtr : sourceIt->second) {

        auto* hex = hexPtr.get();

        if (!hex || !isHexFree(hex))
            continue;

        auto* hexTransform = _world->getHexTransformByCell(*hex);

        if (!hexTransform)
            continue;

        bool hasPathNeighbour = false;
        glm::vec3 pathPos;

        for (const auto& neighbourCoord : HexCoord::getNeighbors(hex->getCoord())) {

            auto neighbour = _world->getGrid()->getCell(neighbourCoord);

            if (!neighbour)
                continue;

            if (neighbour->getVisualState() != HexCell::VisualState::Path)
                continue;

            auto* pathTransform = _world->getHexTransformByCell(*neighbour);

            if (!pathTransform)
                continue;

            hasPathNeighbour = true;
            pathPos = pathTransform->getPosition();
            break;
        }

        if (!hasPathNeighbour)
            continue;

        glm::vec3 signPos = hexTransform->getPosition();
        glm::vec3 targetPos = bridgeTransform->getPosition();

        glm::vec2 forward(targetPos.x - pathPos.x, targetPos.z - pathPos.z);

        glm::vec2 side(signPos.x - pathPos.x, signPos.z - pathPos.z);

        float cross = forward.x * side.y - forward.y * side.x;

        if (cross > 0.0f)
            candidates.push_back(hex);
    }

    if (candidates.empty())
        return;

    HexCell* signHex = candidates[rand() % candidates.size()];

    auto* signTransform = _world->getHexTransformByCell(*signHex);

    if (!signTransform)
        return;

    auto* scene = _world->getOwner()->getScene();

    auto* sign = dzemikk::PrefabSerializer::instantiate(*scene, *_cache["Signpost"].get(),
                                                        _assetManager, signTransform->getOwner());

    if (!sign)
        return;

    signHex->setState(HexCell::State::Prop);
    signHex->setVisualState(HexCell::VisualState::Signpost);

    sign->transform()->setPosition(glm::vec3(0.f));

    glm::vec3 dir = glm::normalize(bridgeTransform->getPosition() - signTransform->getPosition());

    float angle = glm::degrees(std::atan2(dir.x, dir.z));

    constexpr float modelOffset = -180.0f;

    glm::quat rot = glm::angleAxis(glm::radians(angle + modelOffset), glm::vec3(0, 0, 1));

    sign->transform()->setRotation(rot);
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

game::HexCell*
game::WorldVisualManager::getExtremeHexOfType(const std::vector<std::shared_ptr<HexCell>>& hexes,
                                              HexCell::Type type, bool top) {
    HexCell* result = nullptr;

    float bestZ = top ? -FLT_MAX : FLT_MAX;

    for (auto& hex : hexes) {

        if (!hex)
            continue;

        if (hex->getType() != type)
            continue;

        auto* t = _world->getHexTransformByCell(*hex);
        if (!t)
            continue;

        float z = t->getPosition().z;

        if (top) {
            if (z > bestZ) {
                bestZ = z;
                result = hex.get();
            }
        } else {
            if (z < bestZ) {
                bestZ = z;
                result = hex.get();
            }
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
        hex->getVisualState() == HexCell::VisualState::Path || hex->getVisualState() == HexCell::VisualState::Path)
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
            neighbor->getType() == HexCell::Type::Bridge ||
            neighbor->getState() == HexCell::State::Item ||
            neighbor->getState() == HexCell::State::Totem ||
            neighbor->getVisualState() == HexCell::VisualState::Signpost)
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

void game::WorldVisualManager::spawnCampChunk(const std::string& chunkName) {
    if (!_world)
        return;

    auto chunks = _world->getVisualHexesByChunk();
    auto chunkIt = chunks.find(chunkName);
    if (chunkIt == chunks.end())
        return;

    auto* scene = _world->getOwner()->getScene();
    const auto& hexes = chunkIt->second;

    std::vector<HexCell*> freeHexes;
    freeHexes.reserve(hexes.size());

    for (auto& hexPtr : hexes) {
        if (!hexPtr)
            continue;

        HexCell* hex = hexPtr.get();

        if (!isHexFree(hex))
            continue;

        auto* t = _world->getHexTransformByCell(*hex);
        if (!t)
            continue;

        freeHexes.push_back(hex);
    }

    if (freeHexes.size() < 3)
        return;

    glm::vec3 center(0.f);
    int count = 0;

    for (auto* hex : freeHexes) {
        auto* t = _world->getHexTransformByCell(*hex);
        if (!t)
            continue;

        center += t->getPosition();
        count++;
    }

    if (count == 0)
        return;

    center /= static_cast<float>(count);

    std::sort(freeHexes.begin(), freeHexes.end(), [&](HexCell* a, HexCell* b) {
        auto* ta = _world->getHexTransformByCell(*a);
        auto* tb = _world->getHexTransformByCell(*b);

        if (!ta || !tb)
            return false;

        float da = glm::length2(ta->getPosition() - center);
        float db = glm::length2(tb->getPosition() - center);

        return da < db;
    });

    HexCell* centerHex = freeHexes[0];
    auto* centerT = _world->getHexTransformByCell(*centerHex);

    if (!centerT)
        return;

    glm::vec3 centerPos = centerT->getPosition();

    auto* fire =
        dzemikk::PrefabSerializer::instantiate(*scene, *_cache["Camp_fire"].get(), _assetManager);

    if (fire) {
        fire->transform()->setPosition(centerPos);
        fire->setName("CampFire_" + chunkName);
        centerHex->setState(HexCell::State::Prop);
    }

    std::vector<HexCell*> ringHexes;

    HexCoord centerCoord = centerHex->getCoord();

    constexpr int minRing = 3;
    constexpr int maxRing = 3;

    for (HexCell* hex : freeHexes) {

        int dist = HexCoord::distance(centerCoord, hex->getCoord());

        if (dist < minRing || dist > maxRing)
            continue;

        ringHexes.push_back(hex);
    }

    if (ringHexes.size() < 3)
        ringHexes = freeHexes;

    std::shuffle(ringHexes.begin(), ringHexes.end(), std::mt19937(std::random_device{}()));

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> rotOffsetDist(-20.0f, 20.0f);

    int tipiCount = 1 + rand() % 3;
    int spawned = 0;

    std::vector<HexCell*> selectedHexes;

    for (HexCell* hex : ringHexes) {

        if (spawned >= tipiCount)
            break;

        if (!hex)
            continue;

        bool isNeighbour = false;

        for (auto* selected : selectedHexes) {
            if (HexCoord::distance(selected->getCoord(), hex->getCoord()) <= 1) {
                isNeighbour = true;
                break;
            }
        }

        if (isNeighbour)
            continue;

        auto* t = _world->getHexTransformByCell(*hex);
        if (!t)
            continue;

        glm::vec3 pos = t->getPosition();

        auto* tipi =
            dzemikk::PrefabSerializer::instantiate(*scene, *_cache["Tipi"].get(), _assetManager);

        if (!tipi)
            continue;

        tipi->transform()->setPosition(pos);

        glm::vec3 dir = glm::normalize(centerPos - pos);

        float yaw = std::atan2(dir.x, dir.z);

        glm::quat rotY = glm::angleAxis(yaw + 90, glm::vec3(0, 1, 0));
        glm::quat rotX = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0));

        tipi->transform()->setRotation(rotY * rotX);

        hex->setState(HexCell::State::Prop);

        selectedHexes.push_back(hex);
        spawned++;
    }

    spdlog::info(
        "[WorldVisualManager] Camp spawned in chunk '{}' (tipis: {}, spawned: {}, ring: {}-{})",
        chunkName, tipiCount, spawned, minRing, maxRing);
}