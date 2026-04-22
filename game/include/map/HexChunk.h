#ifndef GAME_HEXCHUNK_H
#define GAME_HEXCHUNK_H

#include "map/HexCoord.h"

#include <functional>
#include <vector>

namespace game {
class Perlin;

class HexChunk {
  public:
    struct Config {
        int steps{0};
        float holeChance{10.0F};
        std::function<float(int)> generator;
        std::function<bool(const HexCoord&)> canPlace;
    };

    HexChunk() = default;
    HexChunk(HexCoord center, const Config& config);
    HexChunk(int radius, HexCoord center, Perlin* perlin, float holeProbability);

    [[nodiscard]] const std::vector<HexCoord>& getHexes() const;
    [[nodiscard]] int getRadius() const;
    [[nodiscard]] HexCoord getCenter() const;
    [[nodiscard]] const Config& getConfig() const;

    void remove(const std::vector<HexCoord>& hexes);
    [[nodiscard]] std::vector<HexCoord> intersection(const HexChunk& other) const;

  private:
    static std::vector<HexCoord> generateHexes(HexCoord center, const Config& config);
    static std::vector<HexCoord> generateHexes(int radius, HexCoord center, Perlin* perlin,
                                               float holeProbability);

    std::vector<HexCoord> _hexes;
    int _radius{0};
    HexCoord _center{0, 0};
    Config _config{};
};
} // namespace game

#endif // GAME_HEXCHUNK_H
