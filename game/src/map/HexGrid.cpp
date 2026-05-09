#include "map/HexGrid.h"

#include "boost/uuid/detail/nil_uuid.hpp"
#include "boost/uuid/uuid.hpp"
#include "utils/perlin.h"

#include <algorithm>
#include <ranges>
#include <spdlog/spdlog.h>
#include <unordered_set>
#include <utility>

namespace game {
HexGrid::HexGrid(std::mt19937& rng) : _rng(rng) {}

std::pair<HexCoord, HexCoord> HexGrid::closestPair(boost::uuids::uuid idx1,
                                                   boost::uuids::uuid idx2) {
    const auto& chunk1 = std::ranges::find_if(
        _chunks, [&idx1](const HexChunk& chunk) { return chunk.getId() == idx1; });
    const auto& chunk2 = std::ranges::find_if(
        _chunks, [&idx2](const HexChunk& chunk) { return chunk.getId() == idx2; });

    std::pair<HexCoord, HexCoord> closest{chunk1->getHexes().begin()->first,
                                          chunk2->getHexes().begin()->first};
    int minDist = std::numeric_limits<int>::max();
    for (const auto& [coord1, cell1] : chunk1->getHexes()) {
        for (const auto& [coord2, cell2] : chunk2->getHexes()) {
            auto dist = game::HexCoord::distance(coord1, coord2);
            if (dist < minDist) {
                minDist = dist;
                closest = {coord1, coord2};
            }
        }
    }
    return closest;
}

boost::uuids::uuid HexGrid::makeChunk(const HexChunk::Config& config) {
    HexChunk chunk(config, {0, 0});

    if (chunk.getHexes().empty()) {
        return boost::uuids::nil_uuid();
    }

    Perlin perlinHoles(2);
    std::vector<HexCoord> coordsToRemove;
    for (const auto& [coord, cell] : chunk.getHexes()) {
        float noiseVal = perlinHoles.noise(static_cast<float>(coord.q()) * 0.1F,
                                           static_cast<float>(coord.r()) * 0.1F);
        if (noiseVal < config.holeChance && cell.getType() == HexCell::Type::Normal) {
            coordsToRemove.push_back(coord);
        }
    }
    chunk.remove(coordsToRemove);

    if (chunk.getHexes().empty()) {
        return boost::uuids::nil_uuid();
    }

    _chunks.emplace_back(std::move(chunk));
    return _chunks.back().getId();
}

boost::uuids::uuid HexGrid::makeChunk(boost::uuids::uuid parentChunkId, HexCoord::Direction dir,
                                      const HexChunk::Config& config) {
    if (parentChunkId == boost::uuids::nil_uuid()) {
        return boost::uuids::nil_uuid();
    }

    const auto& parentChunk =
        std::ranges::find_if(_chunks, [&parentChunkId](const HexChunk& chunk) {
            return chunk.getId() == parentChunkId;
        });

    HexChunk chunk(config, parentChunk->getOrigin());
    const auto* chunkHexes = &chunk.getHexes();

    if (chunkHexes->empty()) {
        return boost::uuids::nil_uuid();
    }

    Perlin perlinHoles(2);
    std::vector<HexCoord> coordsToRemove;
    for (const auto& [coord, cell] : *chunkHexes) {
        float noiseVal = perlinHoles.noise(static_cast<float>(coord.q()) * 0.1F,
                                           static_cast<float>(coord.r()) * 0.1F);
        if (noiseVal < config.holeChance && cell.getType() == HexCell::Type::Normal) {
            coordsToRemove.push_back(coord);
        }
    }
    chunk.remove(coordsToRemove);

    if (chunkHexes->empty()) {
        return boost::uuids::nil_uuid();
    }

    // Post-porcessing -> Move new chunk towards parent chunk until they are 1 hex apart
    // Connect the chunks with a single-hex bridge

    auto dirToParent = HexCoord::dir(HexCoord::dir(dir).opposite());
    if (dirToParent.has_value()) {
        chunk.setDirToParent(dirToParent.value());
    }

    // Initial shift to avoid immediate intersection
    chunk.shift(dir, parentChunk->getConfig().steps + config.steps + 2);

    while (parentChunk->intersection(chunk, true).empty()) {
        chunk.shift(dirToParent.value());
    }

    chunk.shift(dir, 2);

    _chunks.push_back(std::move(chunk));

    auto closest = closestPair(parentChunkId, _chunks.back().getId());
    auto bridgeCoord = HexCoord::hexesOnLine(closest.first, closest.second);

    if (!bridgeCoord.empty()) {
        for (const auto& coord : bridgeCoord) {
            _chunks.back().assignCell({coord, HexCell::State::Empty, HexCell::Type::Normal});
        }
    }

    return _chunks.back().getId();
}
} // namespace game
