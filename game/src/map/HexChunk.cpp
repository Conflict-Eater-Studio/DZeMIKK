#include "map/HexChunk.h"

#include "map/HexCoord.h"
#include "utils/Perlin.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <ranges>
#include <unordered_set>

namespace game {
HexChunk::HexChunk(HexCoord center, const Config& config)
    : _hexes(generateHexes(center, config)), _radius(config.steps), _center(center),
      _config(config) {
    if (!_config.generator) {
        _config.generator = [steps = _config.steps](int x) {
            if (steps <= 0) {
                return 0.0F;
            }
            return 1.0F - (static_cast<float>(x) / static_cast<float>(steps));
        };
    }
}

HexChunk::HexChunk(int radius, HexCoord center, Perlin* perlin, float holeProbability)
    : _hexes(generateHexes(radius, center, perlin, holeProbability)), _radius(radius),
      _center(center),
      _config({.steps = radius, .holeChance = holeProbability, .generator = [radius](int x) {
                   if (radius <= 0) {
                       return 0.0F;
                   }
                   return 1.0F - (static_cast<float>(x) / static_cast<float>(radius));
               }}) {}

std::vector<HexCoord> HexChunk::generateHexes(HexCoord center, const Config& config) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> chanceDist(0.0F, 1.0F);

    std::unordered_set<HexCoord> visited;
    std::vector<HexCoord> hexes;

    auto canPlace = config.canPlace;
    if (!canPlace) {
        canPlace = [](const HexCoord&) { return true; };
    }

    if (!canPlace(center)) {
        return hexes;
    }

    visited.insert(center);
    hexes.push_back(center);

    std::vector<HexCoord> currentGen = {center};

    auto generator = config.generator;
    if (!generator) {
        generator = [steps = config.steps](int x) {
            if (steps <= 0) {
                return 0.0F;
            }
            return 1.0F - (static_cast<float>(x) / static_cast<float>(steps));
        };
    }

    for (int x = 0; x < config.steps; x++) {
        std::vector<HexCoord> nextGen;
        const float chance = generator(x);

        for (const auto& coord : currentGen) {
            auto neighbors = HexCoord::getNeighbors(coord);

            for (const auto& neighborCoord : neighbors) {
                if (visited.contains(neighborCoord)) {
                    continue;
                }

                if (!canPlace(neighborCoord)) {
                    continue;
                }

                if (chanceDist(rng) < chance) {
                    visited.insert(neighborCoord);
                    nextGen.push_back(neighborCoord);
                    hexes.push_back(neighborCoord);
                }
            }
        }

        currentGen = std::move(nextGen);
        if (currentGen.empty()) {
            break;
        }
    }

    return hexes;
}

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

const HexChunk::Config& HexChunk::getConfig() const {
    return _config;
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
