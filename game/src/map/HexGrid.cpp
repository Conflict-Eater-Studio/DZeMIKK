#include "map/HexGrid.h"

#include "boost/uuid/detail/nil_uuid.hpp"
#include "boost/uuid/uuid.hpp"

#include <algorithm>
#include <limits>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace game {
HexGrid::HexGrid(unsigned int seed) : _rng(seed), _seed(seed) {}

HexGrid::~HexGrid() {
    _chunks.clear();
}

bool HexGrid::isBlockedCell(const HexGrid::HexCellPtr& cell) {
    return cell != nullptr && cell->getGenState() == HexCell::GenState::Blocked;
}

bool HexGrid::isWalkableCell(const HexGrid::HexCellPtr& cell) {
    return cell != nullptr && cell->getGenState() != HexCell::GenState::Blocked;
}

std::pair<HexCoord, HexCoord> HexGrid::closestPair(HexChunk* chunk1, HexChunk* chunk2) {
    auto pickFallbackCoord = [](HexChunk* chunk) {
        bool hasAny = false;
        HexCoord fallback(0, 0);

        for (const auto& [coord, cell] : chunk->getHexes()) {
            if (!hasAny || coord < fallback) {
                fallback = coord;
                hasAny = true;
            }
        }

        return fallback;
    };

    std::pair<HexCoord, HexCoord> closest{pickFallbackCoord(chunk1), pickFallbackCoord(chunk2)};
    int minDist = std::numeric_limits<int>::max();

    for (const auto& [coord1, cell1] : chunk1->getHexes()) {
        if (isBlockedCell(cell1)) {
            continue;
        }

        for (const auto& [coord2, cell2] : chunk2->getHexes()) {
            if (isBlockedCell(cell2)) {
                continue;
            }

            auto dist = HexCoord::distance(coord1, coord2);
            if (dist < minDist ||
                (dist == minDist && (coord1 < closest.first ||
                                     (coord1 == closest.first && coord2 < closest.second)))) {
                minDist = dist;
                closest = {coord1, coord2};
            }
        }
    }

    return closest;
}

bool HexGrid::neighboursChunk(const HexCoord& coord, const boost::uuids::uuid& chunkToSkip) const {
    for (const auto& [otherChunkId, otherChunk] : _chunks) {
        if (otherChunkId == chunkToSkip) {
            continue;
        }

        if (isWalkableCell(otherChunk->getCell(coord))) {
            return true;
        }

        for (const auto& neighbor : HexCoord::getNeighbors(coord)) {
            if (isWalkableCell(otherChunk->getCell(neighbor))) {
                return true;
            }
        }
    }

    return false;
}

void HexGrid::removeOverlaps(HexChunk& chunk) const {
    std::vector<HexCoord> toRemove;

    for (const auto& [coord, cell] : chunk.getHexes()) {
        for (const auto& [existingChunkId, existingChunk] : _chunks) {
            auto existingCell = existingChunk->getCell(coord);
            if (existingCell != nullptr) {
                toRemove.push_back(coord);
                break;
            }
        }
    }

    if (!toRemove.empty()) {
        chunk.remove(toRemove);
    }
}

void HexGrid::makeBridge(const boost::uuids::uuid& parentChunkId, const boost::uuids::uuid& chunkId,
                         const std::pair<HexCoord, HexCoord>& closest) {
    auto hexes = HexCoord::hexesOnLine(closest.first, closest.second);

    for (const auto& hex : hexes) {
        HexChunk* owningChunk = nullptr;

        if (_chunks[parentChunkId]->contains(hex)) {
            owningChunk = _chunks[parentChunkId].get();
        } else if (_chunks[chunkId]->contains(hex)) {
            owningChunk = _chunks[chunkId].get();
        } else {
            for (const auto& [otherId, otherChunk] : _chunks) {
                if (otherId == parentChunkId || otherId == chunkId) {
                    continue;
                }
                if (otherChunk->contains(hex)) {
                    owningChunk = otherChunk.get();
                    break;
                }
            }
        }

        if (owningChunk != nullptr) {
            owningChunk->getCell(hex)->setGenState(HexCell::GenState::Protected);
            owningChunk->getCell(hex)->setType(HexCell::Type::Bridge);
        } else {
            _chunks[chunkId]->assignCell(std::make_shared<HexCell>(
                hex, HexCell::State::Empty, HexCell::Type::Bridge, HexCell::GenState::Protected));
        }
    }

    _chunks[parentChunkId]->protectPathToOrigin(closest.first);
    _chunks[chunkId]->protectPathToOrigin(closest.second);
}

void HexGrid::cleanChunkBorders(const boost::uuids::uuid& chunkId) {
    auto& chunk = _chunks.at(chunkId);
    std::vector<HexCoord> toRemove;

    for (const auto& [coord, cell] : chunk->getHexes()) {
        if (cell->getGenState() == HexCell::GenState::Blocked ||
            cell->getType() == HexCell::Type::Bridge) {
            continue;
        }

        if (neighboursChunk(coord, chunkId)) {
            toRemove.push_back(coord);
        }
    }

    if (!toRemove.empty()) {
        chunk->remove(toRemove);
    }
}

void HexGrid::removeUnreachableHexes() {
    if (_rootChunkId == boost::uuids::nil_uuid()) {
        return;
    }

    auto rootIt = _chunks.find(_rootChunkId);
    if (rootIt == _chunks.end()) {
        return;
    }

    auto origin = rootIt->second->getOrigin();
    auto originCell = getCell(origin);
    if (!isWalkableCell(originCell)) {
        return;
    }

    std::unordered_set<HexCoord> visited;
    std::queue<HexCoord> frontier;
    visited.insert(origin);
    frontier.push(origin);

    while (!frontier.empty()) {
        auto current = frontier.front();
        frontier.pop();

        for (const auto& neighbor : HexCoord::getNeighbors(current)) {
            if (visited.contains(neighbor)) {
                continue;
            }

            HexCellPtr neighborCell = nullptr;
            for (const auto& [chunkId, chunk] : _chunks) {
                auto cell = chunk->getCell(neighbor);
                if (isWalkableCell(cell)) {
                    neighborCell = std::move(cell);
                    break;
                }
            }

            if (neighborCell == nullptr) {
                continue;
            }

            visited.insert(neighbor);
            frontier.push(neighbor);
        }
    }

    for (const auto& [chunkId, chunk] : _chunks) {
        for (const auto& [coord, cell] : chunk->getHexes()) {
            if (isWalkableCell(cell) && !visited.contains(coord)) {
                cell->setGenState(HexCell::GenState::Blocked);
            }
        }
    }
}

void HexGrid::clean() {
    if (_cleaned) {
        return;
    }
    _cleaned = true;
    for (const auto& [chunkId, chunk] : _chunks) {
        chunk->clean();
    }
}

boost::uuids::uuid HexGrid::makeChunk(const HexChunk::Config& config) {
    auto parentChunkId = config.parentChunkId;
    bool hasParent = parentChunkId != boost::uuids::nil_uuid();

    if (!hasParent && !_chunks.empty()) {
        return boost::uuids::nil_uuid();
    }

    std::unique_ptr<HexChunk> chunk = nullptr;
    if (!hasParent) {
        chunk = std::make_unique<HexChunk>(config, _seed);
    } else {
        auto parentIt = _chunks.find(parentChunkId);
        if (parentIt == _chunks.end()) {
            return boost::uuids::nil_uuid();
        }
        chunk = std::make_unique<HexChunk>(config, parentIt->second.get(), _seed);
    }

    if (chunk == nullptr || chunk->getHexes().empty()) {
        return boost::uuids::nil_uuid();
    }

    removeOverlaps(*chunk);
    if (chunk->getHexes().empty()) {
        return boost::uuids::nil_uuid();
    }

    auto chunkId = chunk->getId();
    _chunks.insert({chunkId, std::move(chunk)});

    if (!hasParent) {
        _rootChunkId = chunkId;
    }

    if (hasParent) {
        auto closest = closestPair(_chunks[parentChunkId].get(), _chunks[chunkId].get());
        makeBridge(parentChunkId, chunkId, closest);
        cleanChunkBorders(chunkId);
    }

    removeUnreachableHexes();
    return chunkId;
}

HexGrid::HexCellPtr HexGrid::getCell(const HexCoord& coord) const {
    HexCellPtr selectedCell = nullptr;
    boost::uuids::uuid selectedChunkId = boost::uuids::nil_uuid();

    for (const auto& [chunkId, chunk] : _chunks) {
        auto cell = chunk->getCell(coord);
        if (cell == nullptr) {
            continue;
        }

        if (selectedCell == nullptr || chunkId < selectedChunkId) {
            selectedCell = std::move(cell);
            selectedChunkId = chunkId;
        }
    }

    return selectedCell;
}

HexGrid::HexCellPtr HexGrid::at(const HexCoord& coord) const {
    auto cell = getCell(coord);
    if (cell == nullptr) {
        throw std::out_of_range("HexCell not found for given coordinate");
    }

    return cell;
}

bool HexGrid::contains(const HexCoord& coord) const {
    return getCell(coord) != nullptr;
}

std::vector<HexGrid::HexCellPtr> HexGrid::findPath(const HexCellPtr& startCell,
                                                   const HexCellPtr& targetCell) const {
    if (startCell == nullptr || targetCell == nullptr) {
        return {};
    }

    const HexCoord start = startCell->getCoord();
    const HexCoord target = targetCell->getCoord();

    if (!isWalkableCell(startCell) || !isWalkableCell(targetCell)) {
        return {};
    }

    struct PathNode {
        HexCoord coord;
        int fScore;

        bool operator<(const PathNode& other) const {
            return fScore > other.fScore;
        }
    };

    std::priority_queue<PathNode> frontier;
    frontier.push({start, HexCoord::distance(start, target)});
    std::unordered_map<HexCoord, HexCoord> cameFrom;

    std::unordered_map<HexCoord, int> gScore;
    std::unordered_set<HexCoord> closedSet;

    gScore[start] = 0;

    while (!frontier.empty()) {
        const HexCoord current = frontier.top().coord;
        frontier.pop();

        if (closedSet.contains(current)) {
            continue;
        }

        if (current == target) {
            std::vector<HexCellPtr> path;
            HexCoord pathCoord = target;

            while (pathCoord != start) {
                auto cell = getCell(pathCoord);
                if (cell == nullptr) {
                    return {};
                }

                path.push_back(cell);
                pathCoord = cameFrom.at(pathCoord);
            }

            path.push_back(startCell);
            std::reverse(path.begin(), path.end());
            return path;
        }

        closedSet.insert(current);

        for (const auto& neighbor : HexCoord::getNeighbors(current)) {
            if (closedSet.contains(neighbor)) {
                continue;
            }

            auto neighborCell = getCell(neighbor);
            if (!isWalkableCell(neighborCell)) {
                continue;
            }
            if (targetCell->getType() != HexCell::Type::EnemyBattleHex) {
                if (neighborCell.get()->getType() == HexCell::Type::EnemyBattleHex) {
                    continue;
                }
            }
            if (neighborCell->getState() == HexCell::State::Prop) {
                continue;
            }
            if (neighborCell->getEntity() != nullptr && neighbor != target) {
                continue;
            }

            const int neighbourGScore = gScore.at(current) + 1;
            auto neighborGScoreIt = gScore.find(neighbor);

            if (neighborGScoreIt != gScore.end() && neighbourGScore >= neighborGScoreIt->second) {
                continue;
            }

            cameFrom[neighbor] = current;
            gScore[neighbor] = neighbourGScore;

            const int fScore = neighbourGScore + HexCoord::distance(neighbor, target);
            frontier.push({neighbor, fScore});
        }
    }

    return {};
}

bool HexGrid::moveCell(const HexCoord& from, const HexCoord& to) {
    if (from == to || contains(to)) {
        return false;
    }

    HexChunk* sourceChunk = nullptr;
    for (const auto& [chunkId, chunk] : _chunks) {
        if (chunk->contains(from)) {
            sourceChunk = chunk.get();
            break;
        }
    }

    if (sourceChunk == nullptr) {
        return false;
    }

    auto cell = sourceChunk->extractCell(from);
    if (cell == nullptr) {
        return false;
    }

    if (!sourceChunk->insertCell(to, cell)) {
        sourceChunk->insertCell(from, std::move(cell));
        return false;
    }

    return true;
}

HexGrid::HexCellPtr HexGrid::findCellByEntity(Entity* entity) const {
    for (const auto& [chunkId, chunk] : _chunks) {
        for (const auto& [coord, cell] : chunk->getHexes()) {
            if (cell && cell->getEntity() == entity) {
                return cell;
            }
        }
    }
    return nullptr;
}
} // namespace game
