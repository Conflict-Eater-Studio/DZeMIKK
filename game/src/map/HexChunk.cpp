#include "map/HexChunk.h"

#include "map/hexCoord.h"
#include "utils/Perlin.h"

#include <algorithm>
#include <cmath>
#include <ranges>
#include <unordered_set>

namespace game {
HexChunk::HexChunk(int radius, HexCoord center, Perlin* perlin, float holeProbability)
    : _hexes(generateHexes(radius, center, perlin, holeProbability)), _radius(radius),
      _center(center) {}

std::vector<HexCoord> HexChunk::generateHexes(int radius, HexCoord center, Perlin* perlin,
                                              float holeProbability) {
    std::vector<HexCoord> hexes;
    hexes.reserve(static_cast<std::size_t>((3LL * radius * radius) + (3LL * radius) + 1LL));

    float holeFrequency = 0.15F;
    float heightFrequency = 0.08F;
    float maxHeight = 2.0F;

    for (int q = -radius; q <= radius; q++) {
        int rStart = std::max(-radius, -q - radius);
        int rEnd = std::min(radius, -q + radius);

        for (int r = rStart; r <= rEnd; r++) {
            HexCoord local(q, r);
            HexCoord world = center + local;

            float holeVal = perlin->noise(static_cast<float>(world.q()) * holeFrequency,
                                          static_cast<float>(world.r()) * holeFrequency);

            float normalizedHole = (holeVal + 1.0F) / 2.0F;

            if (normalizedHole < holeProbability && HexCoord::distance(local, HexCoord(0, 0)) > 3) {
                continue;
            }

            float heightVal =
                perlin->noise((static_cast<float>(world.q()) + 100.0F) * heightFrequency,
                              (static_cast<float>(world.r()) + 100.0F) * heightFrequency);

            world.setHeight(heightVal * maxHeight);

            hexes.push_back(world);
        }
    }
    return hexes;
}
const std::vector<HexCoord>& HexChunk::getHexes() const {
    return _hexes;
}

int HexChunk::getRadius() const {
    return _radius;
}

HexCoord HexChunk::getCenter() const {
    return _center;
}

void HexChunk::remove(const std::vector<HexCoord>& hexes) {
    std::unordered_set<HexCoord> removeSet(hexes.begin(), hexes.end());
    std::erase_if(_hexes, [&removeSet](const HexCoord& h) { return removeSet.contains(h); });
}

std::vector<HexCoord> HexChunk::intersection(const HexChunk& other) const {
    if (HexCoord::distance(_center, other._center) > (_radius + other._radius)) {
        return {};
    }

    const std::vector<HexCoord>& smaller =
        (_hexes.size() < other._hexes.size()) ? _hexes : other._hexes;
    const std::vector<HexCoord>& larger =
        (_hexes.size() < other._hexes.size()) ? other._hexes : _hexes;

    std::vector<HexCoord> result;
    result.reserve(smaller.size());

    std::unordered_set<HexCoord> lookupSet;
    lookupSet.reserve(larger.size());
    lookupSet.insert(larger.begin(), larger.end());

    for (const auto& hex : smaller) {
        if (lookupSet.contains(hex)) {
            result.push_back(hex);
        }
    }

    return result;
}
} // namespace game
