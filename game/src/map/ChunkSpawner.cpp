#include "map/ChunkSpawner.h"

#include "map/HexChunk.h"
#include "map/HexCoord.h"
#include "utils/perlin.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

namespace game {
ChunkSpawner::ChunkSpawner(HexCoord rootCenter, int rootRadius, unsigned int seed)
    : _perlin(std::make_unique<Perlin>(seed)) {
    _chunks.emplace_back(rootRadius, rootCenter, _perlin.get(), _config.holeProbability);
}

const std::vector<HexChunk>& ChunkSpawner::getChunks() const {
    return _chunks;
}

std::optional<std::size_t> ChunkSpawner::trySpawnChunk(std::size_t parentChunkIndex,
                                                       HexCoord::Direction dir, int chunkRadius) {
    const auto& parent = _chunks.at(parentChunkIndex);
    auto dist = (parent.getRadius() + chunkRadius + 2) / 2;
    HexCoord offset = HexCoord::dir(dir) * dist;
    HexCoord nextCenter = parent.getCenter() + offset;
    HexChunk nextChunk =
        HexChunk(chunkRadius + 1, nextCenter, _perlin.get(), _config.holeProbability);

    const auto intersection = nextChunk.intersection(parent);

    if ((float)intersection.size() >= (float)nextChunk.getHexes().size() * 0.5F) {
#if DZEMIKK_DEV_TOOLS
        spdlog::info("Cannot make chunk when 50% or more of it's tiles would be overlapping");
#endif
        return -1;
    };

    nextChunk.remove(intersection);

    _chunks.emplace_back(nextChunk);
    return _chunks.size() - 1;
}
} // namespace game
