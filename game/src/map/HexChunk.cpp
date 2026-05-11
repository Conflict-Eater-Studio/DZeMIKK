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
    generateHexes();
}

void HexChunk::setDirToParent(HexCoord::Direction dir) {
    _dirToParent = dir;
}

void HexChunk::generateHexes() {
    for (int q = -_config.steps; q <= _config.steps; q++) {
        int rStart = std::max(-_config.steps, -q - _config.steps);
        int rEnd = std::min(_config.steps, -q + _config.steps);

        for (int r = rStart; r <= rEnd; r++) {
            HexCoord coord{q, r};
            coord += _origin;
            _hexes.emplace(coord, HexCell{coord, HexCell::State::Empty, HexCell::Type::Normal});
        }
    }
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
