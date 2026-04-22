#ifndef GAME_GRID_H
#define GAME_GRID_H

#include "map/HexChunk.h"

#include <cstddef>
#include <optional>
#include <set>
#include <tuple>
#include <vector>

namespace game {
class Grid {
  public:
    enum class CellState : uint8_t { Empty, Occupied, Blocked };

    struct Cell {
        HexCoord coord;
        CellState state{CellState::Empty};

        bool operator<(const Cell& other) const {
            return std::make_tuple(coord.q(), coord.r()) <
                   std::make_tuple(other.coord.q(), other.coord.r());
        }
    };

    Grid() = default;

    [[nodiscard]] std::set<Cell> getHexes() const {
        std::set<Cell> occupied;
        for (const auto& cell : _cells) {
            if (cell.state == CellState::Occupied) {
                occupied.insert(cell);
            }
        }
        return occupied;
    }

    [[nodiscard]] const std::vector<HexChunk>& getChunks() const {
        return _chunks;
    }

    std::optional<std::size_t> makeChunk(HexCoord center, const HexChunk::Config& config);
    std::optional<std::size_t> makeChunk(std::size_t parentChunkIndex, HexCoord::Direction dir,
                                         const HexChunk::Config& config);

    [[nodiscard]] const std::set<Cell>& getCells() const {
        return _cells;
    }

  private:
    void markChunk(const HexChunk& chunk);

    std::set<Cell> _cells;
    std::vector<HexChunk> _chunks;
};
} // namespace game

#endif // GAME_GRID_H
