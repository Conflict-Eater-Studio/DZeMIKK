#include "map/grid.h"

#include <unordered_set>
#include <utility>

namespace game {
std::optional<std::size_t> Grid::makeChunk(HexCoord center, const HexChunk::Config& config) {
    auto constrainedConfig = config;
    constrainedConfig.canPlace = [this](const HexCoord& coord) {
        return !_cells.contains(Cell{.coord = coord});
    };

    HexChunk chunk(center, constrainedConfig);

    if (chunk.getHexes().empty()) {
        return false;
    }

    _chunks.push_back(std::move(chunk));
    markChunk(_chunks.back());
    return true;
}

std::optional<std::size_t> Grid::makeChunk(std::size_t parentChunkIndex, HexCoord::Direction dir,
                                           const HexChunk::Config& config) {
    if (parentChunkIndex >= _chunks.size()) {
        return std::nullopt;
    }

    const auto& parentChunk = _chunks.at(parentChunkIndex);
    const int dist = parentChunk.getConfig().steps + config.steps;
    const HexCoord nextCenter = parentChunk.getCenter() + HexCoord::dir(dir) * dist;

    auto constrainedConfig = config;
    constrainedConfig.canPlace = [this](const HexCoord& coord) {
        return !_cells.contains(Cell{.coord = coord});
    };

    HexChunk chunk(nextCenter, constrainedConfig);

    if (chunk.getHexes().empty()) {
        return std::nullopt;
    }

    _chunks.push_back(std::move(chunk));
    markChunk(_chunks.back());
    return _chunks.size() - 1;
}

void Grid::markChunk(const HexChunk& chunk) {
    std::unordered_set<HexCoord> chunkHexes;
    chunkHexes.reserve(chunk.getHexes().size());
    chunkHexes.insert(chunk.getHexes().begin(), chunk.getHexes().end());

    for (const auto& hex : chunk.getHexes()) {
        _cells.insert(Cell{.coord = hex, .state = CellState::Occupied});
    }

    for (const auto& hex : chunk.getHexes()) {
        for (const auto& neighbor : HexCoord::getNeighbors(hex)) {
            if (chunkHexes.contains(neighbor)) {
                continue;
            }

            auto it = _cells.find(Cell{.coord = neighbor});
            if (it == _cells.end()) {
                _cells.insert(Cell{.coord = neighbor, .state = CellState::Blocked});
            }
        }
    }
}
} // namespace game
