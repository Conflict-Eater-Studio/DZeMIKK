#ifndef GAME_GRID_H
#define GAME_GRID_H

#include "map/HexChunk.h"
#include "map/gridcell.h"

#include <cstddef>
#include <optional>
#include <random>
#include <set>
#include <vector>

namespace game {
class Grid {
  public:
    Grid() = default;
    Grid(std::mt19937& rng);

    [[nodiscard]] std::set<GridCell> getHexes() const {
        return _cells;
    }

    [[nodiscard]] const std::vector<HexChunk>& getChunks() const {
        return _chunks;
    }

    std::optional<std::size_t> makeChunk(HexCoord center, const HexChunk::Config& config);
    std::optional<std::size_t> makeChunk(std::size_t parentChunkIndex, HexCoord::Direction dir,
                                         const HexChunk::Config& config);

  private:
    std::set<GridCell> _cells;
    std::vector<HexChunk> _chunks;
    std::mt19937 _rng;

    std::pair<HexCoord, HexCoord> closestPair(std::size_t idx1, std::size_t idx2);
};
} // namespace game

#endif // GAME_GRID_H
