#ifndef GAME_SAVE_SNAPSHOT_H
#define GAME_SAVE_SNAPSHOT_H

#include "map/HexCoord.h"

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace game {

class SaveSnapshot {
  public:
    struct CellData {
        HexCoord coord;
        uint8_t state = 0;
        uint8_t type = 0;
        uint8_t genState = 0;
        float height = 0.0F;
        bool checkpoint = false;
        bool checkpointUsed = false;
    };

    struct ChunkSnapshot {
        nlohmann::json config;
        HexCoord origin;
        std::unordered_map<std::string, CellData> cells;
        nlohmann::json bridges;
        nlohmann::json blockingPatterns;
    };

    struct ItemEntry {
        nlohmann::json config;
        HexCoord gridPos;
    };

    struct EnemyEntry {
        nlohmann::json config;
        HexCoord gridPos;
        std::string chunkId;
    };

    struct TotemEntry {
        nlohmann::json config;
        HexCoord gridPos;
    };

    void buildFull(const nlohmann::json& fullSave);

    void updateCell(const std::string& chunkKey, const CellData& data);
    void addChunkSnapshot(const std::string& chunkKey, const ChunkSnapshot& chunk);

    void removeItem(const std::string& persistantId);
    void removeEnemy(const std::string& persistantId);
    void updateTotemUsed(const std::string& persistantId, bool used);

    void setDialogs(const nlohmann::json& dialogs);
    void setPlayer(const nlohmann::json& player);

    [[nodiscard]] nlohmann::json serialize() const;
    [[nodiscard]] bool isValid() const {
        return _valid;
    }
    void invalidate();

  private:
    static std::string coordKey(const HexCoord& c);

    std::unordered_map<std::string, ChunkSnapshot> _chunks;
    std::unordered_map<std::string, ItemEntry> _items;
    std::unordered_map<std::string, EnemyEntry> _enemies;
    std::unordered_map<std::string, TotemEntry> _totems;
    nlohmann::json _dialogs;
    nlohmann::json _player;
    bool _valid = false;
};

} // namespace game

#endif // GAME_SAVE_SNAPSHOT_H
