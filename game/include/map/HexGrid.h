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

    [[nodiscard]] const std::unordered_map<boost::uuids::uuid, std::unique_ptr<HexChunk>>&
    getChunks() const {
        return _chunks;
    }

    boost::uuids::uuid makeChunk(const HexChunk::Config& config);
    HexChunk* findChunkForCoord(const HexCoord& coord);
    [[nodiscard]] HexCellPtr getCell(const HexCoord& coord) const;
    [[nodiscard]] HexCellPtr at(const HexCoord& coord) const;
    [[nodiscard]] bool contains(const HexCoord& coord) const;
    bool moveCell(const HexCoord& from, const HexCoord& to);
    HexCellPtr findCellByEntity(Entity* entity) const;
    void clean();

    [[nodiscard]] bool isChunkBlocked(const boost::uuids::uuid& chunkId) const;
    [[nodiscard]] const BlockingPatternInfo*
    getBlockingPatternInfo(const boost::uuids::uuid& chunkId) const;
    [[nodiscard]] const BlockingPatternInfo*
    findBlockingPatternByCoord(const HexCoord& coord) const;
    bool unlockChunk(const boost::uuids::uuid& chunkId);

    [[nodiscard]] const std::unordered_map<std::pair<boost::uuids::uuid, boost::uuids::uuid>,
                                           BridgeInfo>&
    getBridges() const;
    void lockBridge(std::pair<boost::uuids::uuid, boost::uuids::uuid> bridgeId,
                    const boost::uuids::uuid& enemyId);
    void unlockBridge(std::pair<boost::uuids::uuid, boost::uuids::uuid> bridgeId,
                      const boost::uuids::uuid& enemyId);

  private:
    static bool isBlockedCell(const HexCellPtr& cell);
    static bool isWalkableCell(const HexCellPtr& cell);
    static bool isReachableCell(const HexCellPtr& cell);
    std::unordered_map<boost::uuids::uuid /*HexChunk ID*/, std::unique_ptr<HexChunk>> _chunks;
    std::unordered_map<std::pair<boost::uuids::uuid /*Parent*/, boost::uuids::uuid /*Child*/>,
                       BridgeInfo>
        _bridges;
    std::unordered_map<boost::uuids::uuid /*Blocked Chunk ID*/, BlockingPatternInfo>
        _blockingPatterns;
    unsigned int _seed;
    std::mt19937 _rng;
    boost::uuids::uuid _rootChunkId;
    bool _cleaned{false};

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
};
} // namespace game

#endif // GAME_GRID_H
