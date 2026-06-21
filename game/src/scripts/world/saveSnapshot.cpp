#include "scripts/world/saveSnapshot.h"

#include <chrono>
#include <format>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

namespace game {

std::string SaveSnapshot::coordKey(const HexCoord& c) {
    return std::format("{},{}", c.q(), c.r());
}

void SaveSnapshot::buildFull(const nlohmann::json& fullSave) {
#if DZEMIKK_DEV_TOOLS
    auto totalStart = std::chrono::steady_clock::now();
#endif

    _chunks.clear();
    _items.clear();
    _enemies.clear();
    _totems.clear();
    _dialogs = nlohmann::json();
    _player = nlohmann::json();

#if DZEMIKK_DEV_TOOLS
    auto gridStart = std::chrono::steady_clock::now();
#endif

    if (fullSave.contains("chunkData")) {
        for (const auto& [chunkKey, chunkJson] : fullSave["chunkData"].items()) {
            ChunkSnapshot chunk;

            if (chunkJson.contains("config")) {
                chunk.config = chunkJson["config"];
            }
            if (chunkJson.contains("origin")) {
                chunk.origin = chunkJson["origin"].get<HexCoord>();
            }
            if (chunkJson.contains("cells")) {
                for (const auto& cellJson : chunkJson["cells"]) {
                    CellData cell;
                    cell.coord = cellJson.at("coord").get<HexCoord>();
                    cell.state = cellJson.at("state").get<uint8_t>();
                    cell.type = cellJson.at("type").get<uint8_t>();
                    cell.genState = cellJson.at("genState").get<uint8_t>();
                    cell.height = cellJson.at("height").get<float>();
                    cell.checkpoint = cellJson.at("checkpoint").get<bool>();
                    cell.checkpointUsed = cellJson.at("checkpointUsed").get<bool>();
                    chunk.cells[coordKey(cell.coord)] = cell;
                }
            }
            if (chunkJson.contains("bridges")) {
                chunk.bridges = chunkJson["bridges"];
            }
            if (chunkJson.contains("blockingPatterns")) {
                chunk.blockingPatterns = chunkJson["blockingPatterns"];
            }

            _chunks[chunkKey] = std::move(chunk);
        }
    }

#if DZEMIKK_DEV_TOOLS
    auto gridEnd = std::chrono::steady_clock::now();
    auto gridUs = std::chrono::duration_cast<std::chrono::microseconds>(gridEnd - gridStart).count();
    spdlog::info("[SaveSnapshot::buildFull] Grid parse: {} us", gridUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto itemsStart = std::chrono::steady_clock::now();
#endif

    if (fullSave.contains("items")) {
        for (const auto& [id, itemJson] : fullSave["items"].items()) {
            ItemEntry entry;
            entry.config = itemJson;
            if (itemJson.contains("gridPos")) {
                entry.gridPos = itemJson["gridPos"].get<HexCoord>();
            }
            _items[id] = std::move(entry);
        }
    }

#if DZEMIKK_DEV_TOOLS
    auto itemsEnd = std::chrono::steady_clock::now();
    auto itemsUs = std::chrono::duration_cast<std::chrono::microseconds>(itemsEnd - itemsStart).count();
    spdlog::info("[SaveSnapshot::buildFull] Items parse: {} us", itemsUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto enemiesStart = std::chrono::steady_clock::now();
#endif

    if (fullSave.contains("enemies")) {
        for (const auto& [id, enemyJson] : fullSave["enemies"].items()) {
            EnemyEntry entry;
            entry.config = enemyJson;
            if (enemyJson.contains("gridPos")) {
                entry.gridPos = enemyJson["gridPos"].get<HexCoord>();
            }
            if (enemyJson.contains("chunkId")) {
                entry.chunkId = enemyJson["chunkId"].get<std::string>();
            }
            _enemies[id] = std::move(entry);
        }
    }

#if DZEMIKK_DEV_TOOLS
    auto enemiesEnd = std::chrono::steady_clock::now();
    auto enemiesUs = std::chrono::duration_cast<std::chrono::microseconds>(enemiesEnd - enemiesStart).count();
    spdlog::info("[SaveSnapshot::buildFull] Enemies parse: {} us", enemiesUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto totemsStart = std::chrono::steady_clock::now();
#endif

    if (fullSave.contains("totems")) {
        for (const auto& [id, totemJson] : fullSave["totems"].items()) {
            TotemEntry entry;
            entry.config = totemJson;
            if (totemJson.contains("gridPos")) {
                entry.gridPos = totemJson["gridPos"].get<HexCoord>();
            }
            _totems[id] = std::move(entry);
        }
    }

#if DZEMIKK_DEV_TOOLS
    auto totemsEnd = std::chrono::steady_clock::now();
    auto totemsUs = std::chrono::duration_cast<std::chrono::microseconds>(totemsEnd - totemsStart).count();
    spdlog::info("[SaveSnapshot::buildFull] Totems parse: {} us", totemsUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto dialogsStart = std::chrono::steady_clock::now();
#endif

    if (fullSave.contains("dialogs")) {
        _dialogs = fullSave["dialogs"];
    }

#if DZEMIKK_DEV_TOOLS
    auto dialogsEnd = std::chrono::steady_clock::now();
    auto dialogsUs = std::chrono::duration_cast<std::chrono::microseconds>(dialogsEnd - dialogsStart).count();
    spdlog::info("[SaveSnapshot::buildFull] Dialogs parse: {} us", dialogsUs);
#endif

#if DZEMIKK_DEV_TOOLS
    auto playerStart = std::chrono::steady_clock::now();
#endif

    if (fullSave.contains("player")) {
        _player = fullSave["player"];
    }

#if DZEMIKK_DEV_TOOLS
    auto playerEnd = std::chrono::steady_clock::now();
    auto playerUs = std::chrono::duration_cast<std::chrono::microseconds>(playerEnd - playerStart).count();
    spdlog::info("[SaveSnapshot::buildFull] Player parse: {} us", playerUs);
#endif

    _valid = true;

#if DZEMIKK_DEV_TOOLS
    auto totalEnd = std::chrono::steady_clock::now();
    auto totalUs = std::chrono::duration_cast<std::chrono::microseconds>(totalEnd - totalStart).count();
    spdlog::info("[SaveSnapshot::buildFull] Total: {} us", totalUs);
#endif
}

void SaveSnapshot::updateCell(const std::string& chunkKey, const CellData& data) {
    auto it = _chunks.find(chunkKey);
    if (it == _chunks.end()) {
        return;
    }
    it->second.cells[coordKey(data.coord)] = data;
}

void SaveSnapshot::addChunkSnapshot(const std::string& chunkKey, const ChunkSnapshot& chunk) {
    _chunks[chunkKey] = chunk;
}

void SaveSnapshot::removeItem(const std::string& persistantId) {
    _items.erase(persistantId);
}

void SaveSnapshot::removeEnemy(const std::string& persistantId) {
    _enemies.erase(persistantId);
}

void SaveSnapshot::updateTotemUsed(const std::string& persistantId, bool used) {
    auto it = _totems.find(persistantId);
    if (it == _totems.end()) {
        return;
    }
    it->second.config["used"] = used;
}

void SaveSnapshot::setDialogs(const nlohmann::json& dialogs) {
    _dialogs = dialogs;
}

void SaveSnapshot::setPlayer(const nlohmann::json& player) {
    _player = player;
}

void SaveSnapshot::invalidate() {
    _valid = false;
}

nlohmann::json SaveSnapshot::serialize() const {
    nlohmann::json j;

    for (const auto& [chunkKey, chunk] : _chunks) {
        if (!chunk.config.is_null()) {
            j["chunkData"][chunkKey]["config"] = chunk.config;
        }
        j["chunkData"][chunkKey]["origin"] = chunk.origin;

        auto& cellsArr = j["chunkData"][chunkKey]["cells"];
        cellsArr = nlohmann::json::array();
        for (const auto& [key, cell] : chunk.cells) {
            nlohmann::json data{
                {"coord", cell.coord},
                {"state", cell.state},
                {"type", cell.type},
                {"genState", cell.genState},
                {"height", cell.height},
                {"checkpoint", cell.checkpoint},
                {"checkpointUsed", cell.checkpointUsed},
            };
            cellsArr.emplace_back(std::move(data));
        }

        if (!chunk.bridges.is_null() && !chunk.bridges.empty()) {
            j["chunkData"][chunkKey]["bridges"] = chunk.bridges;
        }
        if (!chunk.blockingPatterns.is_null() && !chunk.blockingPatterns.empty()) {
            j["chunkData"][chunkKey]["blockingPatterns"] = chunk.blockingPatterns;
        }
    }

    if (!_items.empty()) {
        for (const auto& [id, entry] : _items) {
            j["items"][id] = entry.config;
            j["items"][id]["gridPos"] = entry.gridPos;
        }
    }

    if (!_enemies.empty()) {
        for (const auto& [id, entry] : _enemies) {
            j["enemies"][id] = entry.config;
            j["enemies"][id]["gridPos"] = entry.gridPos;
            j["enemies"][id]["chunkId"] = entry.chunkId;
        }
    }

    if (!_totems.empty()) {
        for (const auto& [id, entry] : _totems) {
            j["totems"][id] = entry.config;
            j["totems"][id]["gridPos"] = entry.gridPos;
        }
    }

    if (!_dialogs.is_null()) {
        j["dialogs"] = _dialogs;
    }

    if (!_player.is_null()) {
        j["player"] = _player;
    }

    return j;
}

} // namespace game
