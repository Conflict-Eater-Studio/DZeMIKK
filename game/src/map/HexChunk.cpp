#include "map/HexChunk.h"

#include "boost/uuid/random_generator.hpp"
#include "map/Entity.h"

#include <algorithm>
#include <limits>
#include <queue>
#include <unordered_set>
#include <utility>
#include <vector>

namespace game {
HexChunk::HexChunk(HexChunk::Config config, HexChunk* parent)
    : _parent(parent), _config(std::move(config)), _id(boost::uuids::random_generator_mt19937()()) {

    auto dist = ((parent->getConfig().steps + _config.steps) / 2) + 2;
    _origin = parent->getOrigin() + HexCoord::dir(_config.dirFromParent) * dist;

    if (!_config.generator) {
        _config.generator = [steps = _config.steps](int x) {
            if (steps <= 0) {
                return 0.0F;
            }
            return x % 2 == 0 ? 1.0F : 1.0F - (static_cast<float>(x) / static_cast<float>(steps));
            // return 1.0F;
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
            return x % 2 == 0 ? 1.0F : 1.0F - (static_cast<float>(x) / static_cast<float>(steps));
            // return 1.0F;
        };
    }
    generateHexes();
}

void HexChunk::generateHexes() {
    generateHexCells();

    int maxQ = std::numeric_limits<int>::min();
    int minQ = std::numeric_limits<int>::max();
    int maxR = std::numeric_limits<int>::min();
    int minR = std::numeric_limits<int>::max();
    int maxS = std::numeric_limits<int>::min();
    int minS = std::numeric_limits<int>::max();

    for (const auto& entry : _hexes) {
        const auto& coord = entry.first;
        maxQ = std::max(maxQ, coord.q());
        minQ = std::min(minQ, coord.q());
        maxR = std::max(maxR, coord.r());
        minR = std::min(minR, coord.r());
        maxS = std::max(maxS, coord.s());
        minS = std::min(minS, coord.s());
    }

    fillBlockedHexes(minQ - 1, maxQ + 1, minR - 1, maxR + 1, minS - 1, maxS + 1);

    if (_parent != nullptr) {
        while (_parent->intersection(*this).empty()) {
            shift(HexCoord::opposite(_config.dirFromParent), 1);
        }
        shift(_config.dirFromParent, 2);
    }
    blockHexesWithMaxNeighbours(1);
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

void HexChunk::protectPathToOrigin(const HexCoord& start) {
    if (!_hexes.contains(start)) {
        return;
    }

    std::unordered_map<HexCoord, HexCoord> parent;
    std::unordered_set<HexCoord> visited{start};
    std::queue<HexCoord> bfsQueue;
    bfsQueue.push(start);

    bool reachedOrigin = false;
    while (!bfsQueue.empty()) {
        HexCoord current = bfsQueue.front();
        bfsQueue.pop();

        if (current == _origin) {
            reachedOrigin = true;
            break;
        }

        for (const auto& neighbor : HexCoord::getNeighbors(current)) {
            if (!_hexes.contains(neighbor) || visited.contains(neighbor)) {
                continue;
            }
            visited.insert(neighbor);
            parent.emplace(neighbor, current);
            bfsQueue.push(neighbor);
        }
    }

    if (!reachedOrigin) {
        return;
    }

    HexCoord current = _origin;
    while (true) {
        auto it = _hexes.find(current);
        if (it != _hexes.end()) {
            it->second->setGenState(HexCell::GenState::Protected);
        }

        if (current == start) {
            break;
        }

        auto parentIt = parent.find(current);
        if (parentIt == parent.end()) {
            break;
        }
        current = parentIt->second;
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

void HexChunk::blockHexesWithMaxNeighbours(int maxNeighbours) {
    for (const auto& [coord, cell] : _hexes) {
        if (cell->getGenState() != HexCell::GenState::Normal) {
            continue;
        }

        int normalNeighbourCount = 0;
        for (const auto& neighbor : HexCoord::getNeighbors(coord)) {
            auto it = _hexes.find(neighbor);
            if (it != _hexes.end() && it->second->getGenState() == HexCell::GenState::Normal) {
                normalNeighbourCount++;
            }
        }

        if (normalNeighbourCount <= maxNeighbours) {
            cell->setGenState(HexCell::GenState::Blocked);
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
} // namespace game
