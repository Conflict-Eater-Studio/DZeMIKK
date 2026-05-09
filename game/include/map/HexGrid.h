#ifndef GAME_GRID_H
#define GAME_GRID_H

#include "boost/uuid/uuid.hpp"
#include "map/HexCell.h"
#include "map/HexChunk.h"

#include <numeric>
#include <random>
#include <set>
#include <vector>

namespace game {
class HexGrid {
  public:
    HexGrid() = default;
    HexGrid(std::mt19937& rng);

    [[nodiscard]] std::vector<HexCell*> getHexes() {
        std::vector<HexCell*> hexes;
        for (auto& chunk : _chunks) {
            for (auto& [coord, cell] : chunk.getHexes()) {
                hexes.push_back(&cell);
            }
        }
        return hexes;
    }

    [[nodiscard]] const std::vector<HexChunk>& getChunks() const {
        return _chunks;
    }

    HexChunk* update(boost::uuids::uuid uuid) {
        auto it = std::ranges::find_if(
            _chunks, [uuid](const HexChunk& chunk) { return chunk.getId() == uuid; });
        if (it == _chunks.end()) {
            return nullptr;
        }

        return &(*it);
    };

    boost::uuids::uuid makeChunk(const HexChunk::Config& config);
    boost::uuids::uuid makeChunk(boost::uuids::uuid parentChunkId, HexCoord::Direction dir,
                                 const HexChunk::Config& config);

  private:
    std::vector<HexChunk> _chunks;
    std::mt19937 _rng;

    std::pair<HexCoord, HexCoord> closestPair(boost::uuids::uuid idx1, boost::uuids::uuid idx2);
};
} // namespace game

#endif // GAME_GRID_H
