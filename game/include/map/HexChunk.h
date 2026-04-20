#ifndef GAME_HEXCHUNK_H
#define GAME_HEXCHUNK_H

#include "map/HexCoord.h"

#include <vector>

namespace game {
class Perlin;
class HexChunk {
  public:
    HexChunk(int radius, HexCoord center, Perlin* perlin, float holeProbability);

    static std::vector<HexCoord> generateHexes(int radius, HexCoord center, Perlin* perlin,
                                               float holeProbability);

    void remove(const std::vector<HexCoord>& hexes);

    [[nodiscard]] const std::vector<HexCoord>& getHexes() const;
    [[nodiscard]] int getRadius() const;
    [[nodiscard]] HexCoord getCenter() const;

    [[nodiscard]] std::vector<HexCoord> intersection(const HexChunk& other) const;

  private:
    std::vector<HexCoord> _hexes;
    int _radius;
    HexCoord _center;
};
} // namespace game

#endif // GAME_HEXCHUNK_H
