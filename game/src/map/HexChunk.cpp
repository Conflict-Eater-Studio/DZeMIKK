#include "map/HexChunk.h"

#include "boost/uuid/random_generator.hpp"
#include "map/Entity.h"

#include <limits>
#include <queue>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace game {
int axisValue(HexCoord::Direction dir, const HexCoord& coord) {
    switch (dir) {
    case HexCoord::Direction::R0:
    case HexCoord::Direction::R180:
        return coord.q();
    case HexCoord::Direction::R120:
    case HexCoord::Direction::R300:
        return coord.r();
    case HexCoord::Direction::R60:
    case HexCoord::Direction::R240:
        return coord.s();
    default:
        return 0;
    }
}

void updateFurthestEdgeHex(
    const HexCoord& coord, const HexChunk::HexCellPtr& cell, int& minQ, int& maxQ, int& minR,
    int& maxR, int& minS, int& maxS,
    std::unordered_map<HexCoord::Direction, HexChunk::HexCellPtr>& furthestEdgeHexes) {
    int q = coord.q();
    int r = coord.r();
    int s = coord.s();

    if (q > maxQ) {
        maxQ = q;
        furthestEdgeHexes.at(HexCoord::Direction::R0) = cell;
    }
    if (q < minQ) {
        minQ = q;
        furthestEdgeHexes.at(HexCoord::Direction::R180) = cell;
    }
    if (r > maxR) {
        maxR = r;
        furthestEdgeHexes.at(HexCoord::Direction::R120) = cell;
    }
    if (r < minR) {
        minR = r;
        furthestEdgeHexes.at(HexCoord::Direction::R300) = cell;
    }
    if (s > maxS) {
        maxS = s;
        furthestEdgeHexes.at(HexCoord::Direction::R60) = cell;
    }
    if (s < minS) {
        minS = s;
        furthestEdgeHexes.at(HexCoord::Direction::R240) = cell;
    }
}

void alignEdgeTowardParent(
    HexChunk* parent, HexCoord::Direction dirToParent,
    const std::unordered_map<HexCoord, HexChunk::HexCellPtr>& hexes,
    std::unordered_map<HexCoord::Direction, HexChunk::HexCellPtr>& furthestEdgeHexes) {
    HexCoord::Direction oppositeDir = HexCoord::opposite(dirToParent);
    const auto& parentEdges = parent->getFurthestEdgeHexes();
    auto parentIt = parentEdges.find(oppositeDir);
    if (parentIt == parentEdges.end()) {
        return;
    }

    HexCoord parentTarget = parentIt->second->getCoord();
    int extremalValue = axisValue(dirToParent, furthestEdgeHexes.at(dirToParent)->getCoord());

    int bestDist = std::numeric_limits<int>::max();
    HexChunk::HexCellPtr best = nullptr;
    for (const auto& [coord, cell] : hexes) {
        if (axisValue(dirToParent, coord) != extremalValue) {
            continue;
        }
        int dist = HexCoord::distance(coord, parentTarget);
        if (dist < bestDist) {
            bestDist = dist;
            best = cell;
        }
    }

    if (best) {
        furthestEdgeHexes.at(dirToParent) = best;
    }
}

HexChunk::HexChunk(HexChunk::Config config, HexChunk* parent)
    : _parent(parent), _config(std::move(config)), _id(boost::uuids::random_generator_mt19937()()) {

    auto dist = ((parent->getConfig().steps + _config.steps) / 2) + 2;
    _origin = parent->getOrigin() + HexCoord::dir(_config.dirFromParent) * dist;

    if (!_config.generator) {
        _config.generator = [steps = _config.steps](int x) {
            if (steps <= 0) {
                return 0.0F;
            }
            return 1.0F - (static_cast<float>(x) / static_cast<float>(steps * 3));
        };
    }
    generateHexes();
}

HexChunk::HexChunk(HexChunk::Config config)
    : _config(std::move(config)), _id(boost::uuids::random_generator_mt19937()()) {
    if (!_config.generator) {
        _config.generator = [steps = _config.steps](int x) {
            if (steps <= 0) {
                return 0.0F;
            }
            return 1.0F - (static_cast<float>(x) / static_cast<float>(steps * 2));
        };
    }
    generateHexes();
}

void HexChunk::setDirToParent(HexCoord::Direction dir) {
    _dirToParent = dir;
}

void HexChunk::generateHexes() {
    generateHexCells();

    if (_parent != nullptr) {
        while (_parent->intersection(*this, true).empty()) {
            shift(HexCoord::opposite(_config.dirFromParent), 1);
        }
        shift(_config.dirFromParent, 2);
    }

    int minQ = 0;
    int maxQ = 0;
    int minR = 0;
    int maxR = 0;
    int minS = 0;
    int maxS = 0;
    findFurthestEdgeHexes(minQ, maxQ, minR, maxR, minS, maxS);
    auto parent = computeParentMap();
    protectPathsToOrigin(parent);
    fillBlockedHexes(minQ, maxQ, minR, maxR, minS, maxS);
    unblockIsolatedHexes();
}

void HexChunk::generateHexCells() {
    _hexes.insert(
        {_origin, std::make_shared<HexCell>(_origin, HexCell::State::Empty, HexCell::Type::Normal,
                                            HexCell::GenState::Protected)});

    std::unordered_set<HexCoord> visited{_origin};
    std::unordered_set<HexCoord> frontier{_origin};
    for (int i = 0; i < _config.steps; i++) {
        std::unordered_set<HexCoord> nextFrontier;
        for (const auto& coord : frontier) {
            for (const auto& neighbor : HexCoord::getNeighbors(coord)) {
                if (visited.contains(neighbor)) {
                    continue;
                }
                visited.insert(neighbor);
                if (_chanceDist(_rng) < _config.generator(i + 1)) {
                    nextFrontier.insert(neighbor);
                    _hexes.insert(
                        {neighbor, std::make_shared<HexCell>(neighbor, HexCell::State::Empty,
                                                             HexCell::Type::Normal)});
                }
            }
        }
        frontier = std::move(nextFrontier);
    }
}

void HexChunk::findFurthestEdgeHexes(int& minQ, int& maxQ, int& minR, int& maxR, int& minS,
                                     int& maxS) {
    _furthestEdgeHexes = {
        {HexCoord::Direction::R0, _hexes.at(_origin)},
        {HexCoord::Direction::R60, _hexes.at(_origin)},
        {HexCoord::Direction::R120, _hexes.at(_origin)},
        {HexCoord::Direction::R180, _hexes.at(_origin)},
        {HexCoord::Direction::R240, _hexes.at(_origin)},
        {HexCoord::Direction::R300, _hexes.at(_origin)},
    };
    maxQ = std::numeric_limits<int>::min();
    minQ = std::numeric_limits<int>::max();
    maxR = std::numeric_limits<int>::min();
    minR = std::numeric_limits<int>::max();
    maxS = std::numeric_limits<int>::min();
    minS = std::numeric_limits<int>::max();

    for (const auto& [coord, cell] : _hexes) {
        updateFurthestEdgeHex(coord, cell, minQ, maxQ, minR, maxR, minS, maxS, _furthestEdgeHexes);
    }

    if (_parent) {
        alignEdgeTowardParent(_parent, HexCoord::opposite(_config.dirFromParent), _hexes,
                              _furthestEdgeHexes);
    }
}

std::unordered_map<HexCoord, HexCoord> HexChunk::computeParentMap() {
    std::unordered_map<HexCoord, HexCoord> parent;
    std::unordered_set<HexCoord> bfsVisited{_origin};
    std::queue<HexCoord> bfsQueue;
    bfsQueue.push(_origin);
    while (!bfsQueue.empty()) {
        HexCoord current = bfsQueue.front();
        bfsQueue.pop();
        for (const auto& neighbor : HexCoord::getNeighbors(current)) {
            if (!_hexes.contains(neighbor) || bfsVisited.contains(neighbor)) {
                continue;
            }
            bfsVisited.insert(neighbor);
            parent.emplace(neighbor, current);
            bfsQueue.push(neighbor);
        }
    }
    return parent;
}

void HexChunk::protectPathsToOrigin(const std::unordered_map<HexCoord, HexCoord>& parent) {
    for (const auto& [dir, hex] : _furthestEdgeHexes) {
        HexCoord current = hex->getCoord();
        while (current != _origin) {
            auto hexIt = _hexes.find(current);
            if (hexIt != _hexes.end() &&
                hexIt->second->getGenState() != HexCell::GenState::Protected) {
                hexIt->second->setGenState(HexCell::GenState::Protected);
            }
            auto it = parent.find(current);
            if (it == parent.end()) {
                break;
            }
            current = it->second;
        }
    }
}

void HexChunk::fillBlockedHexes(int minQ, int maxQ, int minR, int maxR, int minS, int maxS) {
    for (int q = minQ; q <= maxQ; q++) {
        for (int r = minR; r <= maxR; r++) {
            int s = -q - r;
            if (s < minS || s > maxS) {
                continue;
            }
            HexCoord coord(q, r);
            if (!_hexes.contains(coord)) {
                _hexes.insert({coord, std::make_shared<HexCell>(coord, HexCell::State::Empty,
                                                                HexCell::Type::Normal,
                                                                HexCell::GenState::Blocked)});
            }
        }
    }
}

void HexChunk::unblockIsolatedHexes() {
    for (const auto& [coord, cell] : _hexes) {
        if (cell->getGenState() != HexCell::GenState::Blocked) {
            continue;
        }
        bool hasBlockedNeighbour = false;
        for (const auto& neighbor : HexCoord::getNeighbors(coord)) {
            auto it = _hexes.find(neighbor);
            if (it != _hexes.end() && it->second->getGenState() == HexCell::GenState::Blocked) {
                hasBlockedNeighbour = true;
                break;
            }
        }
        if (!hasBlockedNeighbour) {
            cell->setGenState(HexCell::GenState::Normal);
        }
    }
}

const std::unordered_map<HexCoord, HexChunk::HexCellPtr>& HexChunk::getHexes() const {
    return _hexes;
}

std::unordered_map<HexCoord, HexChunk::HexCellPtr>& HexChunk::getHexes() {
    return _hexes;
}

HexChunk::HexCellPtr HexChunk::getCell(const HexCoord& coord) const {
    auto it = _hexes.find(coord);
    if (it == _hexes.end()) {
        return nullptr;
    }
    return it->second;
}

bool HexChunk::contains(const HexCoord& coord) const {
    return _hexes.contains(coord);
}

HexChunk::HexCellPtr HexChunk::extractCell(const HexCoord& coord) {
    auto it = _hexes.find(coord);
    if (it == _hexes.end()) {
        return nullptr;
    }

    auto cell = std::move(it->second);
    _hexes.erase(it);
    return cell;
}

bool HexChunk::insertCell(const HexCoord& coord, HexCellPtr cell) {
    if (!cell || _hexes.contains(coord)) {
        return false;
    }

    cell->setCoord(coord);
    _hexes.emplace(coord, std::move(cell));
    return true;
}

const HexChunk::Config& HexChunk::getConfig() const {
    return _config;
}

void HexChunk::remove(const std::vector<HexCoord>& hexes) {
    std::unordered_set<HexCoord> removeSet(hexes.begin(), hexes.end());
    std::erase_if(_hexes, [&removeSet](const auto& entry) {
        return removeSet.contains(entry.second->getCoord());
    });
}

std::vector<HexCoord> HexChunk::intersection(const HexChunk& other, bool withBlocked) const {
    std::unordered_set<HexCoord> otherCoords;
    otherCoords.reserve(other._hexes.size());

    for (const auto& [key, cell] : other._hexes) {
        if (!withBlocked && cell->getGenState() == HexCell::GenState::Blocked) {
            continue;
        }
        otherCoords.insert(cell->getCoord());
    }

    std::vector<HexCoord> result;
    for (const auto& [key, cell] : _hexes) {
        if (!withBlocked && cell->getGenState() == HexCell::GenState::Blocked) {
            continue;
        }

        if (otherCoords.contains(cell->getCoord())) {
            result.push_back(cell->getCoord());
        }
    }

    return result;
}

void HexChunk::shift(HexCoord::Direction dir, int times) {
    auto offset = HexCoord::dir(dir);
    std::vector<HexCellPtr> shifted;
    shifted.reserve(_hexes.size());
    for (const auto& [coord, cell] : _hexes) {
        cell->setCoord(coord + offset * times);
        shifted.push_back(cell);
    }

    _origin += offset * times;

    _hexes.clear();
    for (const auto& cell : shifted) {
        _hexes.emplace(cell->getCoord(), cell);
    }
}

void HexChunk::assignCell(HexCellPtr cell) {
    _hexes[cell->getCoord()] = std::move(cell);
}

bool HexChunk::setEntity(const HexCoord& coord, HexCell::Type entityType, Entity* entity) {
    if (!_hexes.contains(coord)) {
        return false;
    }

    _hexes[coord]->setType(entityType);
    _hexes[coord]->setEntity(entity);

    return true;
}

const boost::uuids::uuid& HexChunk::getId() const {
    return _id;
}

HexCoord HexChunk::getOrigin() const {
    return _origin;
}

const std::unordered_map<HexCoord::Direction, HexChunk::HexCellPtr>&
HexChunk::getFurthestEdgeHexes() const {
    return _furthestEdgeHexes;
}
} // namespace game
