#include "map/HexChunk.h"

#include "boost/uuid/random_generator.hpp"
#include "map/Entity.h"

#include <random>
#include <unordered_set>
#include <utility>

namespace game {
HexChunk::HexChunk(Config config, HexCoord origin)
    : _config(std::move(config)), _id(boost::uuids::random_generator_mt19937()()), _origin(origin) {
    if (!_config.generator) {
        _config.generator = [steps = _config.steps](int x) {
            if (steps <= 0) {
                return 0.0F;
            }
            return 1.0F - (static_cast<float>(x) / static_cast<float>(steps));
        };
    }
    _hexes = generateHexes();
}

void HexChunk::setDirToParent(HexCoord::Direction dir) {
    _dirToParent = dir;
}

std::unordered_map<HexCoord, HexCell> HexChunk::generateHexes() {
    const int cap = 1 + (3 * _config.steps * (_config.steps + 1));

    std::unordered_set<HexCoord> visited;
    visited.reserve(cap);

    std::unordered_map<HexCoord, HexCell> hexes;
    hexes.reserve(cap);

    auto generator = _config.generator;

    visited.insert(_origin);
    hexes.insert({_origin, {_origin, HexCell::State::Empty, HexCell::Type::Normal}});

    std::vector<HexCoord> currentGen{_origin};
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
            if (_hexes.contains(n)) {
                continue; // already added by another path
            }

            if (_chanceDist(_rng) >= chance) {
                continue;
            }

            visited.insert(n);
            nextGen.push_back(n);
            hexes.insert({n, {n, HexCell::State::Empty, HexCell::Type::Normal}});
        }

        currentGen = std::move(nextGen);
    }

    return hexes;
}

const std::unordered_map<HexCoord, HexCell>& HexChunk::getHexes() const {
    return _hexes;
}

std::unordered_map<HexCoord, HexCell>& HexChunk::getHexes() {
    return _hexes;
}

const HexChunk::Config& HexChunk::getConfig() const {
    return _config;
}

void HexChunk::remove(const std::vector<HexCoord>& hexes) {
    std::unordered_set<HexCoord> removeSet(hexes.begin(), hexes.end());
    std::erase_if(_hexes, [&removeSet](const auto& entry) {
        return removeSet.contains(entry.second.getCoord());
    });
}

std::vector<HexCell> HexChunk::intersection(const HexChunk& other, bool withBlocked) const {
    std::unordered_set<HexCoord> otherCoords;
    otherCoords.reserve(other._hexes.size());

    for (const auto& [key, cell] : other._hexes) {
        if (!withBlocked && cell.getType() == HexCell::Type::Blocked) {
            continue;
        }
        otherCoords.insert(cell.getCoord());
    }

    std::vector<HexCell> result;
    for (const auto& [key, cell] : _hexes) {
        if (!withBlocked && cell.getType() == HexCell::Type::Blocked) {
            continue;
        }

        if (otherCoords.contains(cell.getCoord())) {
            result.push_back(cell);
        }
    }

    return result;
}

void HexChunk::shift(HexCoord::Direction dir, int times) {
    auto offset = HexCoord::dir(dir);
    std::vector<HexCell> shifted;
    shifted.reserve(_hexes.size());
    for (auto [key, cell] : _hexes) {
        cell.setCoord(cell.getCoord() + offset * times);
        shifted.push_back(cell);
    }
    _origin += offset * times;
    _hexes.clear();
    for (const auto& cell : shifted) {
        _hexes.emplace(cell.getCoord(), cell);
    }
}

void HexChunk::assignCell(HexCell cell) {
    _hexes[cell.getCoord()] = cell;
}

bool HexChunk::setEntity(const HexCoord& coord, HexCell::Type entityType, Entity* entity) {
    if (!_hexes.contains(coord)) {
        return false;
    }

    _hexes[coord].setType(entityType);
    _hexes[coord].setEntity(entity);

    return true;
}

const boost::uuids::uuid& HexChunk::getId() const {
    return _id;
}

HexCoord HexChunk::getOrigin() const {
    return _origin;
}

HexCell* HexChunk::updateAt(const HexCoord& coord) {
    if (!_hexes.contains(coord)) {
        return nullptr;
    }
    return &_hexes[coord];
}
} // namespace game
