#ifndef GAME_GRID_H
#define GAME_GRID_H

#include "boost/uuid/uuid.hpp"
#include "map/HexCell.h"
#include "map/HexChunk.h"

#include <memory>
#include <random>

namespace game {
class HexGrid {
  public:
    using HexCellPtr = std::shared_ptr<HexCell>;

    HexGrid() = default;
    HexGrid(std::mt19937& rng);

    [[nodiscard]] const std::unordered_map<boost::uuids::uuid, std::unique_ptr<HexChunk>>&
    getChunks() const {
        return _chunks;
    }

    boost::uuids::uuid makeChunk(const HexChunk::Config& config);
    [[nodiscard]] HexCellPtr getCell(const HexCoord& coord) const;
    [[nodiscard]] HexCellPtr at(const HexCoord& coord) const;
    [[nodiscard]] bool contains(const HexCoord& coord) const;
    bool moveCell(const HexCoord& from, const HexCoord& to);

  private:
    std::unordered_map<boost::uuids::uuid, std::unique_ptr<HexChunk>> _chunks;
    std::mt19937 _rng;

    static std::pair<HexCoord, HexCoord> closestPair(HexChunk* chunk1, HexChunk* chunk2);
};
} // namespace game

#endif // GAME_GRID_H
