#include "map/HexChunk.h"

#include <algorithm>
#include <random>
#include <unordered_set>

namespace game {
HexChunk::HexChunk(HexCoord center, Config config) : _center(center), _config(std::move(config)) {
    if (!_config.generator) {
        _config.generator = [steps = _config.steps](int x) {
            if (steps <= 0) {
                return 0.0F;
            }
            return 1.0F - (static_cast<float>(x) / static_cast<float>(steps));
        };
    }
    _hexes = generateHexes();
    markChunk();
}

void HexChunk::setDirToParent(HexCoord::Direction dir) {
    _dirToParent = dir;
}

std::unordered_set<GridCell> HexChunk::generateHexes() {
    const int cap = 1 + (3 * _config.steps * (_config.steps + 1));

    std::unordered_set<HexCoord> visited;
    visited.reserve(cap);

    std::unordered_set<GridCell> hexes;
    hexes.reserve(cap);

    auto canPlace = _config.canPlace ? _config.canPlace : [](const HexCoord&) { return true; };
    auto generator = _config.generator;

    if (!canPlace(_center)) {
        return hexes;
    }

    visited.insert(_center);
    hexes.insert({.coord = _center, .state = GridCell::State::Occupied});

    std::vector<HexCoord> currentGen{_center};
    currentGen.reserve(32);

    for (int step = 0; step < _config.steps && !currentGen.empty(); ++step) {
        const float chance = generator(step);

        std::unordered_set<HexCoord> candidates;
        candidates.reserve(currentGen.size() * 3); // rough estimate

        // collect unique unvisited neighbors once
        for (const auto& coord : currentGen) {
            for (int i = 0; i < 6; ++i) {
                const HexCoord n = coord + HexCoord::dir(static_cast<HexCoord::Direction>(i * 2));
                if (visited.contains(n)) {
                    continue;
                }
                if (!candidates.insert(n).second) {
                    continue; // already queued this generation
                }
            }
        }

        std::vector<HexCoord> nextGen;
        nextGen.reserve(candidates.size());

        // evaluate each candidate once
        for (const auto& n : candidates) {
            if (!canPlace(n)) {
                continue;
            }
            if (_chanceDist(_rng) >= chance) {
                continue;
            }

            visited.insert(n);
            nextGen.push_back(n);
            hexes.insert({.coord = n, .state = GridCell::State::Occupied});
        }

        currentGen = std::move(nextGen);
    }

    return hexes;
}

void HexChunk::markChunk() {
    std::unordered_set<HexCoord> occupiedCoords;
    occupiedCoords.reserve(_hexes.size());

    for (const auto& cell : _hexes) {
        occupiedCoords.insert(cell.coord);
    }

    std::unordered_set<HexCoord> borderCoords;
    borderCoords.reserve(_hexes.size());

    for (const auto& cell : _hexes) {
        if (cell.state == GridCell::State::Blocked) {
            continue;
        }

        for (const auto& neighbor : HexCoord::getNeighbors(cell.coord)) {
            if (occupiedCoords.contains(neighbor)) {
                continue;
            }

            borderCoords.insert(neighbor);
            occupiedCoords.insert(neighbor);
        }
    }

    for (const auto& coord : borderCoords) {
        _hexes.insert({.coord = coord, .state = GridCell::State::Blocked});
    }
}

const std::unordered_set<GridCell>& HexChunk::getHexes() const {
    return _hexes;
}

HexCoord HexChunk::getCenter() const {
    return _center;
}

const HexChunk::Config& HexChunk::getConfig() const {
    return _config;
}

void HexChunk::remove(const std::vector<HexCoord>& hexes) {
    std::unordered_set<HexCoord> removeSet(hexes.begin(), hexes.end());
    std::erase_if(_hexes, [&removeSet](const GridCell& h) { return removeSet.contains(h.coord); });
}

std::vector<GridCell> HexChunk::intersection(const HexChunk& other, bool withBlocked) const {
    std::unordered_set<HexCoord> otherCoords;
    otherCoords.reserve(other._hexes.size());

    for (const auto& cell : other._hexes) {
        if (!withBlocked && cell.state == GridCell::State::Blocked) {
            continue;
        }
        otherCoords.insert(cell.coord);
    }

    std::vector<GridCell> result;
    for (const auto& cell : _hexes) {
        if (!withBlocked && cell.state == GridCell::State::Blocked) {
            continue;
        }

        if (otherCoords.contains(cell.coord)) {
            result.push_back(cell);
        }
    }

    return result;
}

void HexChunk::shift(HexCoord::Direction dir) {
    auto offset = HexCoord::dir(dir);
    std::vector<GridCell> shifted;
    shifted.reserve(_hexes.size());
    for (auto cell : _hexes) {
        cell.coord += offset;
        shifted.push_back(cell);
    }
    _center += offset;
    _hexes.clear();
    _hexes.insert(shifted.begin(), shifted.end());
}

void HexChunk::assignCell(GridCell cell) {
    _hexes.insert(cell);
}

bool HexChunk::setOnHex(const HexCoord& coord, GridCell::OnHex onHex,
                        const boost::uuids::uuid& entityId) {
    auto it = std::ranges::find_if(_hexes,
                                   [&coord](const GridCell& cell) { return cell.coord == coord; });
    if (it == _hexes.end()) {
        return false;
    }

    GridCell updated = *it;
    updated.onHex = {entityId, onHex};

    _hexes.erase(it);
    _hexes.insert(updated);
    return true;
}
} // namespace game
