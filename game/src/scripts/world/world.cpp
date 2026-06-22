#include "scripts/world/world.h"

#include "assetManager/assetHandle.h"
#include "assetManager/assetmanager.h"
#include "dialog/dialogManager.h"
#include "ecs/components/collider.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/serialize/prefabSerializer.h"
#include "enemySystem/enemyManager.h"
#include "game.h"
#include "healthSystem.h"
#include "item/itemManager.h"
#include "map/PlayerEntity.h"
#include "player/inventory.h"
#include "player/playerPatternComponent.h"
#include "renderer/shader.h"
#include "scripts/world/saveSnapshot.h"
#include "scripts/world/worldHex.h"
#include "totem/totemManager.h"

#include <boost/uuid/detail/nil_uuid.hpp>
#include <boost/uuid/string_generator.hpp>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <future>
#include <memory>
#include <random>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

namespace game {
World::World(unsigned int seed) : _rng(seed), _perlin(seed), _grid(seed) {
    _worldData.seed = seed;
    _worldData.chunks = {};
    _generators["default"] = [](int step, int maxSteps) {
        return 1.0F - (static_cast<float>(step) / static_cast<float>(maxSteps));
    };
}

void World::onDestroy() {
    waitForSaveCompletion();
}

void World::load(const nlohmann::json& def) {
    boost::uuids::string_generator gen;

    _worldData.chunks.clear();
    _spawnedHexes.clear();
    _hexTransforms.clear();
    _reservedTerritory.clear();

    if (!def.contains("chunkData")) {
        return;
    }

    for (const auto& [chunkKey, chunkJson] : def["chunkData"].items()) {
        auto chunkId = gen(chunkKey);

        ChunkData cd;
        cd.childPersistantId = chunkId;

        HexCoord origin(0, 0);
        if (chunkJson.contains("origin")) {
            origin = chunkJson["origin"].get<HexCoord>();
        }

        std::vector<std::shared_ptr<HexCell>> cells;
        if (chunkJson.contains("cells")) {
            for (const auto& cellJson : chunkJson["cells"]) {
                auto coord = cellJson.at("coord").get<HexCoord>();
                auto state = static_cast<HexCell::State>(cellJson.at("state").get<uint8_t>());
                auto type = static_cast<HexCell::Type>(cellJson.at("type").get<uint8_t>());
                auto genState =
                    static_cast<HexCell::GenState>(cellJson.at("genState").get<uint8_t>());

                auto cell = std::make_shared<HexCell>(coord, state, type, genState);

                if (cellJson.contains("height")) {
                    cell->setHeight(cellJson.at("height").get<float>());
                }
                if (cellJson.contains("checkpoint")) {
                    cell->setCheckpoint(cellJson.at("checkpoint").get<bool>());
                }
                if (cellJson.contains("checkpointUsed")) {
                    cell->setCheckpointUsed(cellJson.at("checkpointUsed").get<bool>());
                }

                cells.push_back(cell);
            }
        }

        HexChunk::Config config;
        config.chunkId = chunkId;

        if (chunkJson.contains("config")) {
            const auto& cfg = chunkJson["config"];
            cd.parentPersistantId = gen(cfg.at("parentPersistantId").get<std::string>());
            cd.name = cfg.at("name").get<std::string>();
            cd.steps = cfg.at("steps").get<int>();
            cd.generatorId = cfg.value("generatorId", std::string("default"));
            cd.dirFromParent =
                static_cast<HexCoord::Direction>(cfg.at("dirFromParent").get<uint8_t>());

            if (cfg.contains("unlockPattern") && !cfg["unlockPattern"].is_null()) {
                cd.unlockPattern = cfg["unlockPattern"].get<HexPattern>();
            }

            config.parentChunkId = cd.parentPersistantId;
            config.name = cd.name;
            config.steps = cd.steps;
            config.generatorId = cd.generatorId;
            config.dirFromParent = cd.dirFromParent;
            config.unlockPattern = cd.unlockPattern;
        }

        _worldData.chunks.push_back(cd);

        auto hexChunk = std::make_unique<HexChunk>(config, cells, origin);

        _grid.loadChunk(std::move(hexChunk));
    }

    for (const auto& [chunkKey, chunkJson] : def["chunkData"].items()) {
        if (!chunkJson.contains("bridges")) {
            continue;
        }
        auto parentId = gen(chunkKey);
        for (const auto& bridgeJson : chunkJson["bridges"]) {
            auto childId = gen(bridgeJson.at("childPersistantId").get<std::string>());
            HexGrid::BridgeId bridgeId{parentId, childId};

            HexGrid::BridgeInfo info;
            info.parentId = parentId;
            info.childId = childId;

            if (bridgeJson.contains("hexes")) {
                for (const auto& hexJson : bridgeJson["hexes"]) {
                    auto coord = hexJson.get<HexCell>();
                    auto cell = _grid.getCell(coord.getCoord());
                    if (cell) {
                        info.hexes.insert(cell.get());
                    } else {
                    }
                }
            }

            _grid.loadBridge(bridgeId, std::move(info));
        }
    }

    for (const auto& [chunkKey, chunkJson] : def["chunkData"].items()) {
        if (!chunkJson.contains("blockingPatterns")) {
            continue;
        }
        auto chunkId = gen(chunkKey);
        for (const auto& bpJson : chunkJson["blockingPatterns"]) {
            HexGrid::BlockingPatternInfo bp;
            bp.parentChunkId = gen(bpJson.at("parentPersistantId").get<std::string>());
            bp.blockedChunkId = gen(bpJson.at("childPersistantId").get<std::string>());
            bp.pattern = bpJson.at("pattern").get<HexPattern>();
            bp.occupiedCoords = bpJson.at("coords").get<std::vector<HexCoord>>();
            bp.unlocked = bpJson.at("unlocked").get<bool>();

            _grid.loadBlockingPattern(chunkId, std::move(bp));
        }
    }

    for (const auto& [id, chunk] : _grid.getChunks()) {
        renderChunk(id);
    }
}

void World::despawnHexVisual(const HexCoord& coord) {
    if (!_spawnedHexes.contains(coord)) {
        return;
    }

    auto* scene = _owner->getScene();
    for (auto it = _hexTransforms.begin(); it != _hexTransforms.end(); ++it) {
        auto* transform = *it;
        if (!transform || !transform->getOwner()) {
            continue;
        }
        auto* worldHex = transform->getOwner()->getComponent<WorldHex>();
        if (!worldHex || !worldHex->getHexCell()) {
            continue;
        }
        if (worldHex->getHexCell()->getCoord() == coord) {
            scene->destroyGameObject(transform->getOwner());
            _hexTransforms.erase(it);
            break;
        }
    }

    _spawnedHexes.erase(coord);
}

void World::loadDiff(const nlohmann::json& def) {
    boost::uuids::string_generator gen;

    waitForSaveCompletion();
    _snapshot.invalidate();

    if (!def.contains("chunkData")) {
        return;
    }

    for (const auto& [chunkKey, chunkJson] : def["chunkData"].items()) {
        auto chunkId = gen(chunkKey);
        auto* chunk = _grid.getChunkById(chunkId);
        if (!chunk) {
            continue;
        }

        if (chunkJson.contains("cells")) {
            std::unordered_map<HexCoord, std::shared_ptr<HexCell>> checkpointCells;
            for (const auto& cellJson : chunkJson["cells"]) {
                auto coord = cellJson.at("coord").get<HexCoord>();
                auto state = static_cast<HexCell::State>(cellJson.at("state").get<uint8_t>());
                auto type = static_cast<HexCell::Type>(cellJson.at("type").get<uint8_t>());
                auto genState =
                    static_cast<HexCell::GenState>(cellJson.at("genState").get<uint8_t>());

                auto cell = std::make_shared<HexCell>(coord, state, type, genState);

                if (cellJson.contains("height")) {
                    cell->setHeight(cellJson.at("height").get<float>());
                }
                if (cellJson.contains("checkpoint")) {
                    cell->setCheckpoint(cellJson.at("checkpoint").get<bool>());
                }
                if (cellJson.contains("checkpointUsed")) {
                    cell->setCheckpointUsed(cellJson.at("checkpointUsed").get<bool>());
                }

                checkpointCells[coord] = cell;
            }

            auto& currentHexes = chunk->getHexes();

            std::vector<HexCoord> coordsToRemove;
            for (const auto& [coord, cell] : currentHexes) {
                if (!checkpointCells.contains(coord)) {
                    coordsToRemove.push_back(coord);
                }
            }

            for (const auto& coord : coordsToRemove) {
                chunk->extractCell(coord);
                despawnHexVisual(coord);
            }

            for (const auto& [coord, checkpointCell] : checkpointCells) {
                auto it = currentHexes.find(coord);
                if (it == currentHexes.end()) {
                    chunk->insertCell(coord, checkpointCell);
                    checkpointCell->setParentChunk(chunk);
                    spawnHexVisual(checkpointCell);
                } else {
                    auto& currentCell = it->second;
                    if (currentCell->getState() != checkpointCell->getState()) {
                        currentCell->setState(checkpointCell->getState());
                    }
                    if (currentCell->getType() != checkpointCell->getType()) {
                        currentCell->setType(checkpointCell->getType());
                    }
                    if (currentCell->getGenState() != checkpointCell->getGenState()) {
                        currentCell->setGenState(checkpointCell->getGenState());
                    }
                    if (currentCell->isCheckpoint() != checkpointCell->isCheckpoint()) {
                        currentCell->setCheckpoint(checkpointCell->isCheckpoint());
                    }
                    if (currentCell->isCheckpointUsed() != checkpointCell->isCheckpointUsed()) {
                        currentCell->setCheckpointUsed(checkpointCell->isCheckpointUsed());
                    }
                }
            }
        }

        if (chunkJson.contains("bridges")) {
            auto parentId = gen(chunkKey);
            for (const auto& bridgeJson : chunkJson["bridges"]) {
                auto childId = gen(bridgeJson.at("childPersistantId").get<std::string>());
                HexGrid::BridgeId bridgeId{parentId, childId};

                HexGrid::BridgeInfo info;
                info.parentId = parentId;
                info.childId = childId;

                if (bridgeJson.contains("hexes")) {
                    for (const auto& hexJson : bridgeJson["hexes"]) {
                        auto coord = hexJson.get<HexCell>();
                        auto cell = _grid.getCell(coord.getCoord());
                        if (cell) {
                            info.hexes.insert(cell.get());
                        }
                    }
                }

                _grid.loadBridge(bridgeId, std::move(info));
            }
        }

        if (chunkJson.contains("blockingPatterns")) {
            auto chunkIdForBp = gen(chunkKey);
            for (const auto& bpJson : chunkJson["blockingPatterns"]) {
                HexGrid::BlockingPatternInfo bp;
                bp.parentChunkId = gen(bpJson.at("parentPersistantId").get<std::string>());
                bp.blockedChunkId = gen(bpJson.at("childPersistantId").get<std::string>());
                bp.pattern = bpJson.at("pattern").get<HexPattern>();
                bp.occupiedCoords = bpJson.at("coords").get<std::vector<HexCoord>>();
                bp.unlocked = bpJson.at("unlocked").get<bool>();

                _grid.loadBlockingPattern(chunkIdForBp, std::move(bp));
            }
        }
    }
}

nlohmann::json World::save() {
    nlohmann::json j;

#if DZEMIKK_DEV_TOOLS
    auto gridStart = std::chrono::steady_clock::now();
#endif

    for (const auto& [chunkId, chunk] : _grid.getChunks()) {
        auto chunkKey = boost::uuids::to_string(chunkId);
        j["chunkData"][chunkKey]["cells"] = nlohmann::json::array();
        // NOLINTBEGIN(modernize-type-traits)
        auto chunkData = std::ranges::find_if(
            _worldData.chunks, [&](const auto& data) { return data.childPersistantId == chunkId; });
        // NOLINTEND(modernize-type-traits)
        if (chunkData != _worldData.chunks.end()) {
            j["chunkData"][chunkKey]["config"] = {
                {"parentPersistantId", boost::uuids::to_string(chunkData->parentPersistantId)},
                {"childPersistantId", boost::uuids::to_string(chunkData->childPersistantId)},
                {"name", chunkData->name},
                {"steps", chunkData->steps},
                {"generatorId", chunkData->generatorId},
                {"dirFromParent", static_cast<uint8_t>(chunkData->dirFromParent)},
                {"unlockPattern", chunkData->unlockPattern}};
        }
        j["chunkData"][chunkKey]["origin"] = chunk->getOrigin();
        for (const auto& [coord, cell] : chunk->getHexes()) {
            HexCell::State state = cell->getState();
            if (state == HexCell::State::Prop) {
                state = HexCell::State::Empty;
            }

            nlohmann::json data{
                {"coord", cell->getCoord()},
                {"state", static_cast<uint8_t>(state)},
                {"type", static_cast<uint8_t>(cell->getType())},
                {"genState", static_cast<uint8_t>(cell->getGenState())},
                {"height", cell->getHeight()},
                {"checkpoint", cell->isCheckpoint()},
                {"checkpointUsed", cell->isCheckpointUsed()},
            };
            j["chunkData"][chunkKey]["cells"].emplace_back(data);
        }
    }

    for (const auto& [id, info] : _grid.getBridges()) {
        nlohmann::json d = {
            {"parentPersistantId", boost::uuids::to_string(info.parentId)},
            {"childPersistantId", boost::uuids::to_string(info.childId)},

        };
        for (const auto& hex : info.hexes) {
            d["hexes"].emplace_back(*hex);
        }
        j["chunkData"][boost::uuids::to_string(id.first)]["bridges"].emplace_back(d);
    }

    for (const auto& [childPersistantId, bp] : _grid.getBlockingPatterns()) {
        nlohmann::json d = {
            {"parentPersistantId", boost::uuids::to_string(bp.parentChunkId)},
            {"childPersistantId", boost::uuids::to_string(bp.blockedChunkId)},
            {"pattern", bp.pattern},
            {"coords", bp.occupiedCoords},
            {"unlocked", bp.unlocked},
        };
        j["chunkData"][boost::uuids::to_string(childPersistantId)]["blockingPatterns"].emplace_back(
            d);
    }

#if DZEMIKK_DEV_TOOLS
    auto gridEnd = std::chrono::steady_clock::now();
    auto gridUs = std::chrono::duration_cast<std::chrono::microseconds>(gridEnd - gridStart).count();
    spdlog::info("[World::save] Grid serialization: {} us", gridUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto itemsStart = std::chrono::steady_clock::now();
#endif

    auto* itemManagerGo = _game->getCurrentScene().get()->findGameObjectByTag("ItemManager");
    if (itemManagerGo) {
        auto* im = itemManagerGo->getComponent<ItemManager>();
        j["items"] = im->saveState()["items"];
    } else {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[World] No ItemManager GameObject found during save");
#endif
    }

#if DZEMIKK_DEV_TOOLS
    auto itemsEnd = std::chrono::steady_clock::now();
    auto itemsUs = std::chrono::duration_cast<std::chrono::microseconds>(itemsEnd - itemsStart).count();
    spdlog::info("[World::save] Items serialization: {} us", itemsUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto enemiesStart = std::chrono::steady_clock::now();
#endif

    auto* enemyManagerGo = _game->getCurrentScene().get()->findGameObjectByTag("EnemyManager");
    if (enemyManagerGo) {
        auto* em = enemyManagerGo->getComponent<EnemyManager>();
        j["enemies"] = em->saveState()["enemies"];
    } else {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[World] No EnemyManager GameObject found during save");
#endif
    }

#if DZEMIKK_DEV_TOOLS
    auto enemiesEnd = std::chrono::steady_clock::now();
    auto enemiesUs = std::chrono::duration_cast<std::chrono::microseconds>(enemiesEnd - enemiesStart).count();
    spdlog::info("[World::save] Enemies serialization: {} us", enemiesUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto totemsStart = std::chrono::steady_clock::now();
#endif

    auto* totemManagerGo = _game->getCurrentScene().get()->findGameObjectByTag("TotemManager");
    if (totemManagerGo) {
        auto* tm = totemManagerGo->getComponent<TotemManager>();
        j["totems"] = tm->saveState()["totems"];
    }

#if DZEMIKK_DEV_TOOLS
    auto totemsEnd = std::chrono::steady_clock::now();
    auto totemsUs = std::chrono::duration_cast<std::chrono::microseconds>(totemsEnd - totemsStart).count();
    spdlog::info("[World::save] Totems serialization: {} us", totemsUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto dialogsStart = std::chrono::steady_clock::now();
#endif

    auto* dialogManagerGo = _game->getCurrentScene().get()->findGameObjectByTag("DialogManager");
    if (dialogManagerGo) {
        auto* dm = dialogManagerGo->getComponent<DialogManager>();
        j["dialogs"] = dm->saveState();
    }

#if DZEMIKK_DEV_TOOLS
    auto dialogsEnd = std::chrono::steady_clock::now();
    auto dialogsUs = std::chrono::duration_cast<std::chrono::microseconds>(dialogsEnd - dialogsStart).count();
    spdlog::info("[World::save] Dialogs serialization: {} us", dialogsUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto playerStart = std::chrono::steady_clock::now();
#endif

    auto* playerGO = _game->getCurrentScene().get()->findGameObjectByTag("Player");
    if (playerGO) {
        if (auto* playerEntity = playerGO->getComponent<PlayerEntity>();
            playerEntity && playerEntity->getCell()) {
            j["player"]["position"] = playerEntity->getCell()->getCoord();
        }

        if (auto* healthGO =
                _game->getCurrentScene().get()->findGameObjectByTag("PlayerHealthSystem");
            healthGO) {
            if (auto* health = healthGO->getComponent<HealthSystem>(); health) {
                j["player"]["health"] = health->getCurrentHealth();
                j["player"]["maxHealth"] = health->getMaxHealth();
            }
        }

        if (auto* patternComp = playerGO->getComponent<PlayerPatternComponent>(); patternComp) {
            for (const auto& entry : patternComp->getPatterns()) {
                j["player"]["patterns"].push_back({{"pattern", entry.pattern},
                                                   {"count", entry.count},
                                                   {"maxCount", entry.maxCount}});
            }
        }

        if (auto* inventory = playerGO->getComponent<Inventory>(); inventory) {
            for (const auto& [type, count] : inventory->getItems()) {
                j["player"]["inventory"][std::to_string(static_cast<uint8_t>(type))] = count;
            }
        }
    }

#if DZEMIKK_DEV_TOOLS
    auto playerEnd = std::chrono::steady_clock::now();
    auto playerUs = std::chrono::duration_cast<std::chrono::microseconds>(playerEnd - playerStart).count();
    spdlog::info("[World::save] Player serialization: {} us", playerUs);
#endif

    return j;
}

void World::waitForSaveCompletion() {
    if (_saveFuture.valid()) {
        _saveFuture.wait();
    }
}

void World::updateSnapshot() {
#if DZEMIKK_DEV_TOOLS
    auto totalStart = std::chrono::steady_clock::now();
#endif

#if DZEMIKK_DEV_TOOLS
    auto gridStart = std::chrono::steady_clock::now();
#endif

    for (const auto& [chunkId, chunk] : _grid.getChunks()) {
        if (!chunk->isSaveDirty()) {
            continue;
        }

        auto chunkKey = boost::uuids::to_string(chunkId);

        if (!_snapshot.isValid()) {
            continue;
        }

        for (const auto& [coord, cell] : chunk->getHexes()) {
            if (!cell->isSaveDirty()) {
                continue;
            }

            HexCell::State state = cell->getState();
            if (state == HexCell::State::Prop) {
                state = HexCell::State::Empty;
            }

            SaveSnapshot::CellData cellData;
            cellData.coord = cell->getCoord();
            cellData.state = static_cast<uint8_t>(state);
            cellData.type = static_cast<uint8_t>(cell->getType());
            cellData.genState = static_cast<uint8_t>(cell->getGenState());
            cellData.height = cell->getHeight();
            cellData.checkpoint = cell->isCheckpoint();
            cellData.checkpointUsed = cell->isCheckpointUsed();

            _snapshot.updateCell(chunkKey, cellData);
            cell->clearSaveDirty();
        }

        chunk->clearSaveDirty();
    }

#if DZEMIKK_DEV_TOOLS
    auto gridEnd = std::chrono::steady_clock::now();
    auto gridUs = std::chrono::duration_cast<std::chrono::microseconds>(gridEnd - gridStart).count();
    spdlog::info("[World::updateSnapshot] Grid cells update: {} us", gridUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto itemsStart = std::chrono::steady_clock::now();
#endif

    auto* itemManagerGo = _game->getCurrentScene().get()->findGameObjectByTag("ItemManager");
    if (itemManagerGo) {
        auto* im = itemManagerGo->getComponent<ItemManager>();
        auto consumedIds = im->getAndClearConsumedItemIds();
        for (const auto& id : consumedIds) {
            _snapshot.removeItem(boost::uuids::to_string(id));
        }
    }

#if DZEMIKK_DEV_TOOLS
    auto itemsEnd = std::chrono::steady_clock::now();
    auto itemsUs = std::chrono::duration_cast<std::chrono::microseconds>(itemsEnd - itemsStart).count();
    spdlog::info("[World::updateSnapshot] Items update: {} us", itemsUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto enemiesStart = std::chrono::steady_clock::now();
#endif

    auto* enemyManagerGo = _game->getCurrentScene().get()->findGameObjectByTag("EnemyManager");
    if (enemyManagerGo) {
        auto* em = enemyManagerGo->getComponent<EnemyManager>();
        auto removedIds = em->getAndClearRemovedEnemyIds();
        for (const auto& id : removedIds) {
            _snapshot.removeEnemy(boost::uuids::to_string(id));
        }
    }

#if DZEMIKK_DEV_TOOLS
    auto enemiesEnd = std::chrono::steady_clock::now();
    auto enemiesUs = std::chrono::duration_cast<std::chrono::microseconds>(enemiesEnd - enemiesStart).count();
    spdlog::info("[World::updateSnapshot] Enemies update: {} us", enemiesUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto totemsStart = std::chrono::steady_clock::now();
#endif

    auto* totemManagerGo = _game->getCurrentScene().get()->findGameObjectByTag("TotemManager");
    if (totemManagerGo) {
        auto* tm = totemManagerGo->getComponent<TotemManager>();
        auto changedTotems = tm->getAndClearChangedTotemIds();
        for (const auto& [id, used] : changedTotems) {
            _snapshot.updateTotemUsed(boost::uuids::to_string(id), used);
        }
    }

#if DZEMIKK_DEV_TOOLS
    auto totemsEnd = std::chrono::steady_clock::now();
    auto totemsUs = std::chrono::duration_cast<std::chrono::microseconds>(totemsEnd - totemsStart).count();
    spdlog::info("[World::updateSnapshot] Totems update: {} us", totemsUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto dialogsStart = std::chrono::steady_clock::now();
#endif

    auto* dialogManagerGo = _game->getCurrentScene().get()->findGameObjectByTag("DialogManager");
    if (dialogManagerGo) {
        auto* dm = dialogManagerGo->getComponent<DialogManager>();
        _snapshot.setDialogs(dm->saveState());
    }

#if DZEMIKK_DEV_TOOLS
    auto dialogsEnd = std::chrono::steady_clock::now();
    auto dialogsUs = std::chrono::duration_cast<std::chrono::microseconds>(dialogsEnd - dialogsStart).count();
    spdlog::info("[World::updateSnapshot] Dialogs update: {} us", dialogsUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto playerStart = std::chrono::steady_clock::now();
#endif

    auto* playerGO = _game->getCurrentScene().get()->findGameObjectByTag("Player");
    if (playerGO) {
        nlohmann::json playerData;

        if (auto* playerEntity = playerGO->getComponent<PlayerEntity>();
            playerEntity && playerEntity->getCell()) {
            playerData["position"] = playerEntity->getCell()->getCoord();
        }

        if (auto* healthGO =
                _game->getCurrentScene().get()->findGameObjectByTag("PlayerHealthSystem");
            healthGO) {
            if (auto* health = healthGO->getComponent<HealthSystem>(); health) {
                playerData["health"] = health->getCurrentHealth();
                playerData["maxHealth"] = health->getMaxHealth();
            }
        }

        if (auto* patternComp = playerGO->getComponent<PlayerPatternComponent>(); patternComp) {
            for (const auto& entry : patternComp->getPatterns()) {
                playerData["patterns"].push_back({{"pattern", entry.pattern},
                                                  {"count", entry.count},
                                                  {"maxCount", entry.maxCount}});
            }
        }

        if (auto* inventory = playerGO->getComponent<Inventory>(); inventory) {
            for (const auto& [type, count] : inventory->getItems()) {
                playerData["inventory"][std::to_string(static_cast<uint8_t>(type))] = count;
            }
        }

        _snapshot.setPlayer(playerData);
    }

#if DZEMIKK_DEV_TOOLS
    auto playerEnd = std::chrono::steady_clock::now();
    auto playerUs = std::chrono::duration_cast<std::chrono::microseconds>(playerEnd - playerStart).count();
    spdlog::info("[World::updateSnapshot] Player update: {} us", playerUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto totalEnd = std::chrono::steady_clock::now();
    auto totalUs = std::chrono::duration_cast<std::chrono::microseconds>(totalEnd - totalStart).count();
    spdlog::info("[World::updateSnapshot] Total snapshot update: {} us", totalUs);
#endif
}

void World::saveToFile(const std::string& filename) {
#if DZEMIKK_DEV_TOOLS
    auto totalStart = std::chrono::steady_clock::now();
#endif

    waitForSaveCompletion();

    bool isFirstSave = !_snapshot.isValid();

    if (isFirstSave) {
#if DZEMIKK_DEV_TOOLS
        auto jsonSaveStart = std::chrono::steady_clock::now();
#endif
        auto fullJson = save();
#if DZEMIKK_DEV_TOOLS
        auto jsonSaveEnd = std::chrono::steady_clock::now();
        auto jsonSaveUs = std::chrono::duration_cast<std::chrono::microseconds>(jsonSaveEnd - jsonSaveStart).count();
        spdlog::info("[World::saveToFile] Full JSON save: {} us", jsonSaveUs);
#endif

#if DZEMIKK_DEV_TOOLS
        auto buildSnapshotStart = std::chrono::steady_clock::now();
#endif
        _snapshot.buildFull(fullJson);
#if DZEMIKK_DEV_TOOLS
        auto buildSnapshotEnd = std::chrono::steady_clock::now();
        auto buildSnapshotUs = std::chrono::duration_cast<std::chrono::microseconds>(buildSnapshotEnd - buildSnapshotStart).count();
        spdlog::info("[World::saveToFile] Build snapshot from JSON: {} us", buildSnapshotUs);
#endif

        for (const auto& [chunkId, chunk] : _grid.getChunks()) {
            for (const auto& [coord, cell] : chunk->getHexes()) {
                cell->clearSaveDirty();
            }
            chunk->clearSaveDirty();
        }
    } else {
        updateSnapshot();
    }

    _saveFuture = std::async(std::launch::async, [this, filename]() {
#if DZEMIKK_DEV_TOOLS
        auto serializeStart = std::chrono::steady_clock::now();
#endif
        auto serializedJson = _snapshot.serialize();
#if DZEMIKK_DEV_TOOLS
        auto serializeEnd = std::chrono::steady_clock::now();
        auto serializeUs = std::chrono::duration_cast<std::chrono::microseconds>(serializeEnd - serializeStart).count();
        spdlog::info("[World::saveToFile] Snapshot serialize (worker thread): {} us", serializeUs);

        auto fileStart = std::chrono::steady_clock::now();
#endif
        std::ofstream out(filename);
        out << serializedJson.dump(4);
        out.close();
#if DZEMIKK_DEV_TOOLS
        auto fileEnd = std::chrono::steady_clock::now();
        auto fileUs = std::chrono::duration_cast<std::chrono::microseconds>(fileEnd - fileStart).count();
        spdlog::info("[World::saveToFile] File write (worker thread): {} us", fileUs);
#endif
    });

#if DZEMIKK_DEV_TOOLS
    auto totalEnd = std::chrono::steady_clock::now();
    auto totalUs = std::chrono::duration_cast<std::chrono::microseconds>(totalEnd - totalStart).count();
    spdlog::info("[World::saveToFile] Total main thread time: {} us", totalUs);
#endif
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

            if (cell->getHexCell()->getType() == HexCell::Type::Normal) {
                cell->getOwner()->getComponent<dzemikk::MeshRenderer>()->setMaterial(
                    0, _hexMaterials[HexCell::Type::Normal]);
                color = glm::vec4(0.075, 0.133, 0.290, 1.0F);
            }

            if (cell->getHexCell()->getVisualState() == HexCell::VisualState::Path) {
                cell->getOwner()->getComponent<dzemikk::MeshRenderer>()->setMaterial(
                    0, _hexMaterialsVisualState[HexCell::VisualState::Path]);
                color = glm::vec4(1.000, 0.875, 0.580, 1.0F);
            }

            if (cell->getHexCell()->getType() == HexCell::Type::EnemyBattleHex) {
                if (cell->getHexCell()->getVisualState() ==
                    HexCell::VisualState::LightUpEnemyBattleHex) {
                    cell->getOwner()->getComponent<dzemikk::MeshRenderer>()->setMaterial(
                        0, _hexMaterialsVisualState[HexCell::VisualState::LightUpEnemyBattleHex]);
                    color = glm::vec4(0.173, 0.106, 0.388, 1.0F);
                } else {
                    cell->getOwner()->getComponent<dzemikk::MeshRenderer>()->setMaterial(
                        0, _hexMaterials[HexCell::Type::EnemyBattleHex]);
                    color = glm::vec4(0.1F, 0.0F, 0.4F, 1.0F);
                }
            }

            if (cell->getHexCell()->getType() == HexCell::Type::PlayerBattleHex) {
                cell->getOwner()->getComponent<dzemikk::MeshRenderer>()->setMaterial(
                    0, _hexMaterials[HexCell::Type::PlayerBattleHex]);
                color = glm::vec4(1.0F, 1.0F, 0.0F, 1.0F);
            }

            if (cell->getHexCell()->getType() == HexCell::Type::BlockingPattern) {
                cell->getOwner()->getComponent<dzemikk::MeshRenderer>()->setMaterial(
                    0, _hexMaterials[HexCell::Type::BlockingPattern]);
                color = glm::vec4(0.0F, 1.0F, 0.0F, 1.0F);
            }

            if (cell->getHexCell()->getType() == HexCell::Type::Bridge) {
                cell->getOwner()->getComponent<dzemikk::MeshRenderer>()->setMaterial(
                    0, _hexMaterials[HexCell::Type::Bridge]);
                color = glm::vec4(0.0F, 1.0F, 1.0F, 1.0F);
            }

            if (cell->getHexCell()->getType() == HexCell::Type::BlockingBridge) {
                cell->getOwner()->getComponent<dzemikk::MeshRenderer>()->setMaterial(
                    0, _hexMaterials[HexCell::Type::BlockingBridge]);
                color = glm::vec4(1.0F, 1.0F, 0.0F, 1.0F);
            }

            cell->getOwner()->getComponent<dzemikk::MeshRenderer>()->getMaterial(0)->setAlbedoColor(
                color);
            cell->getHexCell()->setDirty(false);
        }
    }
}

boost::uuids::uuid World::addChunk(const ChunkData& config) {
    _worldData.chunks.push_back(config);
    auto g = _generators.at(config.generatorId);

    if (g == nullptr) {
        throw std::runtime_error(
            std::format("Generator with id '{}' not found", config.generatorId));
    }

    auto id = _grid.makeChunk({.parentChunkId = config.parentPersistantId,
                               .chunkId = config.childPersistantId,
                               .name = config.name,
                               .steps = config.steps,
                               .generatorId = config.generatorId,
                               .generator = g,
                               .dirFromParent = config.dirFromParent,
                               .unlockPattern = config.unlockPattern});

    _worldData.chunks.back().childPersistantId = id;
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
    if (!cell) {
        return;
    }

    spawnHexVisual(cell);
}

bool World::hasHexVisual(const HexCoord& coord) const {
    return _spawnedHexes.contains(coord);
}

dzemikk::Transform* World::getHexTransformByCell(HexCell cell) {
    for (auto* transform : _hexTransforms) {
        if (!transform) {
            continue;
        }

        auto* worldHex = transform->getOwner()->getComponent<WorldHex>();
        if (!worldHex) {
            continue;
        }

        if (worldHex->getHexCell()->getCoord() == cell.getCoord()) {
            return transform;
        }
    }

    return nullptr;
}

void World::spawnHexVisual(const std::shared_ptr<HexCell>& cell) {
    if (!_assetManager) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("[World] Asset Manager not set for spawnVisualHex to run");
#endif
        return;
    }

    if (!_hexModel) {
        _hexModel = _assetManager->get<dzemikk::Model>("models/assets/hexy/hex_wypukly/hex_wypukly.fbx");
    }

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
    cell->setHeight(height + dist(_rng));
    auto worldPos = cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight());
    obj->transform()->setPosition(worldPos);
    obj->transform()->setScale({1.0F, 1.0F, 1.0F});
    obj->transform()->setRotation(glm::angleAxis(glm::radians(-90.0F), glm::vec3{1.0F, 0.0F, 0.0F}));
    auto* meshRenderer = obj->addComponent<dzemikk::MeshRenderer>();
    meshRenderer->setModel(_hexModel);
    meshRenderer->setCullingRadius(60.0F);

    switch (cell->getGenState()) {
    case HexCell::GenState::Blocked:
        meshRenderer->setMaterial(0, _hexMaterialsGenState[HexCell::GenState::Blocked]);
        meshRenderer->getMaterial(0)->setAlbedoColor(glm::vec4(0.2F, 0.2F, 0.2F, 1.0F));
        break;
    case HexCell::GenState::Protected:
        meshRenderer->setMaterial(0, _hexMaterialsGenState[HexCell::GenState::Protected]);
        meshRenderer->getMaterial(0)->setAlbedoColor(glm::vec4(0.2F, 0.5F, 1.0F, 1.0F));
        break;
    case HexCell::GenState::Normal:
        meshRenderer->setMaterial(0, _hexMaterialsGenState[HexCell::GenState::Normal]);
        meshRenderer->getMaterial(0)->setAlbedoColor(glm::vec4(0.075, 0.133, 0.290, 1.0F));
        break;
    }

    if (cell->getVisualState() == HexCell::VisualState::Path) {
        meshRenderer->setMaterial(0, _hexMaterialsVisualState[HexCell::VisualState::Path]);
        meshRenderer->getMaterial(0)->setAlbedoColor(glm::vec4(0.56F, 0.44F, 0.13F, 1.0F));
    }

    if (cell->getType() == HexCell::Type::Bridge) {
        meshRenderer->setMaterial(0, _hexMaterials[HexCell::Type::Bridge]);
        meshRenderer->getMaterial(0)->setAlbedoColor(glm::vec4(0.0F, 1.0F, 0.0F, 1.0F));
    }
    if (cell->getType() == HexCell::Type::EnemyBattleHex) {
        if (cell->getVisualState() == HexCell::VisualState::LightUpEnemyBattleHex) {
            meshRenderer->setMaterial(
                0, _hexMaterialsVisualState[HexCell::VisualState::LightUpEnemyBattleHex]);
            meshRenderer->getMaterial(0)->setAlbedoColor(glm::vec4(0.173, 0.106, 0.388, 1.0F));
        } else {
            meshRenderer->setMaterial(0, _hexMaterials[HexCell::Type::EnemyBattleHex]);
            meshRenderer->getMaterial(0)->setAlbedoColor(glm::vec4(0.1F, 0.0F, 0.4F, 1.0F));
        }
    }

    auto texture = _game->getEngine()->getAssetManager()->get<dzemikk::Texture>(
        "textures/assets/hex_wypukly_BaseColor.png");
    meshRenderer->getMaterial(0)->setAlbedoTexture(texture);
    meshRenderer->getMaterial(0)->setMetallic(0.05F);
    meshRenderer->getMaterial(0)->setRoughness(0.43F);
    meshRenderer->getMaterial(0)->setAO(0.58F);

    meshRenderer->setTransform(obj->transform());
    auto* worldHex = obj->addComponent<WorldHex>();
    worldHex->setHexCell(cell);

    auto* collider = obj->addComponent<dzemikk::Collider>();
    collider->setModel(_hexModel);
    collider->setTransform(obj->transform());
}

void World::setMaterial(std::shared_ptr<dzemikk::Material> material) {
    _material = material;

    _hexMaterials[HexCell::Type::Normal] = _material;

    _hexMaterials[HexCell::Type::EnemyBattleHex] = _material.get()->clone();
    _hexMaterials[HexCell::Type::PlayerBattleHex] = _material.get()->clone();
    _hexMaterials[HexCell::Type::Bridge] = _material.get()->clone();
    _hexMaterials[HexCell::Type::BlockingBridge] = _material.get()->clone();
    _hexMaterials[HexCell::Type::BlockingPattern] = _material.get()->clone();

    _hexMaterialsGenState[HexCell::GenState::Blocked] = _material.get()->clone();
    _hexMaterialsGenState[HexCell::GenState::Protected] = _material.get()->clone();
    _hexMaterialsGenState[HexCell::GenState::Normal] = _material.get()->clone();

    _hexMaterialsVisualState[HexCell::VisualState::Path] = _material.get()->clone();

    auto shader =
        _game->getEngine()->getAssetManager()->get<dzemikk::Shader>("shaders/PBRFresnelGlow");
    _hexMaterialsVisualState[HexCell::VisualState::LightUpEnemyBattleHex] = _material.get()->clone();
    _hexMaterialsVisualState[HexCell::VisualState::LightUpEnemyBattleHex]->setShader(shader);

}

void World::setPlayer(PlayerEntity* playerEntity) {
    if (playerEntity == nullptr) {
        return;
    }

    _player = playerEntity;
}

void World::clearHexVisuals() {
    auto* scene = _owner->getScene();
    for (auto* transform : _hexTransforms) {
        if (transform && transform->getOwner()) {
            scene->destroyGameObject(transform->getOwner());
        }
    }
    _hexTransforms.clear();
    _spawnedHexes.clear();
    _reservedTerritory.clear();
}

std::unordered_map<std::string, std::vector<std::shared_ptr<HexCell>>>
World::getVisualHexesByChunk() const {
    std::unordered_map<std::string, std::vector<std::shared_ptr<HexCell>>> result;

    for (const auto& [chunkId, chunk] : _grid.getChunks()) {
        auto& visuals = result[_grid.getChunkById(chunkId)->getConfig().name];

        for (const auto& [coord, cell] : chunk->getHexes()) {

            if (_spawnedHexes.contains(coord)) {
                visuals.push_back(cell);
            }
        }
    }

    return result;
}
} // namespace game