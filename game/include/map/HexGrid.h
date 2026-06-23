#ifndef GAME_GRID_H
#define GAME_GRID_H

#include "boost/uuid/uuid.hpp"
#include "map/HexCell.h"
#include "map/HexChunk.h"
#include "map/HexPattern.h"

#include <memory>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace std {
template <> struct hash<std::pair<boost::uuids::uuid, boost::uuids::uuid>> {
    size_t operator()(const std::pair<boost::uuids::uuid, boost::uuids::uuid>& p) const noexcept {
        size_t seed = 0;
        auto combine = [&](size_t val) { seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2); };
        combine(hash<boost::uuids::uuid>{}(p.first));
        combine(hash<boost::uuids::uuid>{}(p.second));
        return seed;
    }
};
} // namespace std

namespace game {
class EnemyEntity;

class HexGrid {
  public:
    using HexCellPtr = std::shared_ptr<HexCell>;
    using BridgeId = std::pair<boost::uuids::uuid /*parent*/, boost::uuids::uuid /*child*/>;

    struct BlockingPatternInfo {
        HexPattern pattern;
        boost::uuids::uuid blockedChunkId;
        boost::uuids::uuid parentChunkId;
        std::vector<HexCoord> occupiedCoords;
        bool unlocked{false};
    };

    struct BridgeInfo {
        boost::uuids::uuid parentId;
        boost::uuids::uuid childId;
        std::unordered_set<HexCell*> hexes;
        std::unordered_set<boost::uuids::uuid> blockingEnemies;
    };

    HexGrid(unsigned int seed = 42);
    HexGrid(const HexGrid&) = delete;
    HexGrid(HexGrid&&) = delete;
    HexGrid& operator=(const HexGrid&) = default;
    HexGrid& operator=(HexGrid&&) = default;
    ~HexGrid();

    // --- Cell access ---
    [[nodiscard]] HexCellPtr getCell(const HexCoord& coord) const;
    [[nodiscard]] HexCellPtr at(const HexCoord& coord) const;
    [[nodiscard]] bool contains(const HexCoord& coord) const;
    bool moveCell(const HexCoord& from, const HexCoord& to);
    HexCellPtr findCellByEntity(Entity* entity) const;

    // --- Flag-based cell queries (uses packed _flags for fast filtering) ---
    [[nodiscard]] std::vector<HexCellPtr> findCells(uint32_t mask, uint32_t value) const;
    [[nodiscard]] std::vector<HexCellPtr> findCellsByState(HexCell::State state) const;
    [[nodiscard]] std::vector<HexCellPtr> findCellsByType(HexCell::Type type) const;

    // --- Chunk access ---
    [[nodiscard]] const std::unordered_map<boost::uuids::uuid, std::unique_ptr<HexChunk>>&
    getChunks() const {
        return _chunks;
    }
    [[nodiscard]] HexChunk* getChunkByName(const std::string& name) const;
    HexChunk* getChunkByName(const std::string& name);
    [[nodiscard]] HexChunk* getChunkById(const boost::uuids::uuid& id) const;
    HexChunk* getChunkById(const boost::uuids::uuid& id);
    HexChunk* findChunkForCoord(const HexCoord& coord);

    // --- Bridge management ---
    [[nodiscard]] const std::unordered_map<BridgeId, BridgeInfo>& getBridges() const;
    void lockBridge(const BridgeId& bridgeId, const boost::uuids::uuid& enemyId);
    void unlockBridge(const BridgeId& bridgeId, const boost::uuids::uuid& enemyId);

    // --- Blocking pattern management ---
    [[nodiscard]] bool isChunkBlocked(const boost::uuids::uuid& chunkId) const;
    [[nodiscard]] const BlockingPatternInfo*
    getBlockingPatternInfo(const boost::uuids::uuid& chunkId) const;
    [[nodiscard]] const BlockingPatternInfo*
    findBlockingPatternByCoord(const HexCoord& coord) const;
    bool unlockChunk(const boost::uuids::uuid& chunkId);
    [[nodiscard]] std::unordered_map<boost::uuids::uuid, BlockingPatternInfo>
    getBlockingPatterns() const;

    // --- Generation ---
    boost::uuids::uuid makeChunk(const HexChunk::Config& config);
    void clean();
    void clear();

    // --- Loading ---
    void loadChunk(std::unique_ptr<HexChunk> chunk);
    void loadBridge(const BridgeId& bridgeId, BridgeInfo info);
    void loadBlockingPattern(const boost::uuids::uuid& chunkId, BlockingPatternInfo info);

    // --- Seed / RNG ---
    [[nodiscard]] unsigned int getSeed() const {
        return _seed;
    }
    [[nodiscard]] std::mt19937& getRng() {
        return _rng;
    }

    [[nodiscard]] std::vector<HexCell*> getBridgeCells(const boost::uuids::uuid& parentId,
                                                       const boost::uuids::uuid& childId) const;

  private:
    // --- Generation helpers ---
    static bool isBlockedCell(const HexCellPtr& cell);
    static bool isWalkableCell(const HexCellPtr& cell);
    static bool isReachableCell(const HexCellPtr& cell);
    static std::pair<HexCoord, HexCoord> closestPair(HexChunk* chunk1, HexChunk* chunk2);
    [[nodiscard]] bool neighboursChunk(const HexCoord& coord,
                                       const boost::uuids::uuid& chunkToSkip) const;
    void removeOverlaps(HexChunk& chunk) const;
    void makeBridge(const boost::uuids::uuid& parentChunkId, const boost::uuids::uuid& chunkId,
                    const std::pair<HexCoord, HexCoord>& closest);
    void placeBlockingPattern(const boost::uuids::uuid& parentChunkId,
                              const boost::uuids::uuid& chunkId, const HexPattern& pattern);
    void cleanChunkBorders(const boost::uuids::uuid& chunkId);
    void removeUnreachableHexes();

    // --- Data ---
    unsigned int _seed;
    std::mt19937 _rng;
    boost::uuids::uuid _rootChunkId;
    bool _cleaned{false};

    std::unordered_map<std::string, HexChunk*> _chunkByName;
    std::unordered_map<boost::uuids::uuid, std::unique_ptr<HexChunk>> _chunks;
    std::unordered_map<BridgeId, BridgeInfo> _bridges;
    std::unordered_map<boost::uuids::uuid, BlockingPatternInfo> _blockingPatterns;
};
} // namespace game

#endif // GAME_GRID_H
