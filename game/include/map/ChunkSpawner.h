#ifndef GAME_CHUNKSPAWNER_H
#define GAME_CHUNKSPAWNER_H

#include "map/HexChunk.h"
#include "map/HexCoord.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace game {

class Perlin;

class ChunkSpawner {
  public:
    struct Config {
        int chunkRadius{30};
        float holeProbability{0.25F};
        int requiredContactPoints{4};
        float maxTrimRatio{0.30F};
        int overlapPadding{1};
        float size{1};
        float spacing{0.1F};
    };

    ChunkSpawner(HexCoord rootCenter, int rootRadius, unsigned int seed);
    ChunkSpawner(const Perlin& perlin, const Config& config);

    std::optional<std::size_t> trySpawnChunk(std::size_t parentChunkIndex, HexCoord::Direction dir,
                                             int chunkRadius);

    [[nodiscard]] const std::vector<HexChunk>& getChunks() const;

  private:
    std::unique_ptr<Perlin> _perlin;
    Config _config;
    std::vector<HexChunk> _chunks;
};

} // namespace game

#endif // GAME_CHUNKSPAWNER_H
