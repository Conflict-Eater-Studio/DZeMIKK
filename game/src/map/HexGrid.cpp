#include "map/HexGrid.h"

#include "boost/uuid/detail/nil_uuid.hpp"
#include "boost/uuid/uuid.hpp"
#include "enemySystem/enemyEntity.h"
#include "map/HexCell.h"

#include <algorithm>
#include <limits>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#if DZEMIKK_DEV_TOOLS
#include "spdlog/spdlog.h"
#endif

namespace game {
HexGrid::HexGrid(unsigned int seed) : _rng(seed), _seed(seed) {}

HexGrid::~HexGrid() {
    _chunks.clear();
}

bool HexGrid::isBlockedCell(const HexGrid::HexCellPtr& cell) {
    return cell != nullptr && cell->getGenState() == HexCell::GenState::Blocked;
}

bool HexGrid::isWalkableCell(const HexGrid::HexCellPtr& cell) {
    return cell != nullptr && cell->getGenState() != HexCell::GenState::Blocked &&
           cell->getType() != HexCell::Type::BlockingPattern &&
           cell->getType() != HexCell::Type::BlockingBridge;
}

bool HexGrid::isReachableCell(const HexGrid::HexCellPtr& cell) {
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

    BridgeInfo bridgeInfo{.parentId = parentChunkId, .childId = chunkId, .hexes = {}};

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
            bridgeInfo.hexes.insert(owningChunk->getCell(hex).get());
        } else {
            auto cell = std::make_shared<HexCell>(hex, HexCell::State::Empty, HexCell::Type::Bridge,
                                                  HexCell::GenState::Protected);
            bridgeInfo.hexes.insert(cell.get());
            _chunks[chunkId]->assignCell(cell);
        }
    }

    _chunks[parentChunkId]->protectPathToOrigin(closest.first);
    _chunks[chunkId]->protectPathToOrigin(closest.second);
    _bridges[{parentChunkId, chunkId}] = std::move(bridgeInfo);
}

void HexGrid::placeBlockingPattern(const boost::uuids::uuid& parentChunkId,
                                   const boost::uuids::uuid& chunkId, const HexPattern& pattern) {
    auto closest = closestPair(_chunks[parentChunkId].get(), _chunks[chunkId].get());
    auto bridgeLine = HexCoord::hexesOnLine(closest.first, closest.second);

    std::unordered_set<HexCoord> patternPositions;
    std::vector<HexCoord> occupiedCoords;

    for (const auto& patternHex : pattern.getHexes()) {
        HexCoord worldPos = closest.first + patternHex;
        patternPositions.insert(worldPos);

        auto* parentChunk = _chunks[parentChunkId].get();
        if (parentChunk->contains(worldPos)) {
            auto cell = parentChunk->getCell(worldPos);
            cell->setType(HexCell::Type::BlockingPattern);
            cell->setGenState(HexCell::GenState::Protected);
        } else {
            _chunks[parentChunkId]->assignCell(std::make_shared<HexCell>(
                worldPos, HexCell::State::Empty, HexCell::Type::BlockingPattern,
                HexCell::GenState::Protected));
        }
        occupiedCoords.push_back(worldPos);
    }

    for (const auto& hex : bridgeLine) {
        if (patternPositions.contains(hex)) {
            continue;
        }

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

    _blockingPatterns.emplace(chunkId, BlockingPatternInfo{.pattern = pattern,
                                                           .blockedChunkId = chunkId,
                                                           .parentChunkId = parentChunkId,
                                                           .occupiedCoords = occupiedCoords,
                                                           .unlocked = false});

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
    if (!isReachableCell(originCell)) {
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
                if (isReachableCell(cell)) {
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
            if (isReachableCell(cell) && !visited.contains(coord)) {
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
        if (config.unlockPattern.has_value()) {
            placeBlockingPattern(parentChunkId, chunkId, config.unlockPattern.value());
        } else {
            auto closest = closestPair(_chunks[parentChunkId].get(), _chunks[chunkId].get());
            makeBridge(parentChunkId, chunkId, closest);
        }
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

bool HexGrid::isChunkBlocked(const boost::uuids::uuid& chunkId) const {
    auto it = _blockingPatterns.find(chunkId);
    return it != _blockingPatterns.end() && !it->second.unlocked;
}

const HexGrid::BlockingPatternInfo*
HexGrid::getBlockingPatternInfo(const boost::uuids::uuid& chunkId) const {
    auto it = _blockingPatterns.find(chunkId);
    if (it == _blockingPatterns.end()) {
        return nullptr;
    }
    return &it->second;
}

const HexGrid::BlockingPatternInfo*
HexGrid::findBlockingPatternByCoord(const HexCoord& coord) const {
    for (const auto& [chunkId, info] : _blockingPatterns) {
        for (const auto& occupiedCoord : info.occupiedCoords) {
            if (occupiedCoord == coord) {
                return &info;
            }
        }
    }
    return nullptr;
}

bool HexGrid::unlockChunk(const boost::uuids::uuid& chunkId) {
    auto it = _blockingPatterns.find(chunkId);
    if (it == _blockingPatterns.end() || it->second.unlocked) {
        return false;
    }

    auto& info = it->second;
    auto& parentChunk = _chunks[info.parentChunkId];

    for (const auto& coord : info.occupiedCoords) {
        auto cell = parentChunk->getCell(coord);
        if (cell != nullptr) {
            cell->setType(HexCell::Type::Bridge);
        }
    }

    info.unlocked = true;
    return true;
}

const std::unordered_map<std::pair<boost::uuids::uuid, boost::uuids::uuid>, HexGrid::BridgeInfo>&
HexGrid::getBridges() const {
    return _bridges;
}

void HexGrid::lockBridge(std::pair<boost::uuids::uuid, boost::uuids::uuid> bridgeId,
                         const boost::uuids::uuid& enemyId) {
    // If bridge between two chunks exists, add the enemy to the set of blocking enemies
    // Bridge is only valid of it's vetween two chunks that are next to each other
    if (_bridges.contains(bridgeId)) {
        if (_bridges[bridgeId].blockingEnemies.empty()) {
            for (const auto& cell : _bridges[bridgeId].hexes) {
                if (cell->getType() == HexCell::Type::Bridge) {
                    cell->setType(HexCell::Type::BlockingBridge);
                }
            }
        }

        _bridges[bridgeId].blockingEnemies.insert(enemyId);
    }
}

void HexGrid::unlockBridge(std::pair<boost::uuids::uuid, boost::uuids::uuid> bridgeId,
                           const boost::uuids::uuid& enemyId) {
    if (_bridges.contains(bridgeId) && _bridges[bridgeId].blockingEnemies.contains(enemyId)) {
        _bridges[bridgeId].blockingEnemies.erase(enemyId);

        if (_bridges[bridgeId].blockingEnemies.empty()) {
            for (const auto& cell : _bridges[bridgeId].hexes) {
                if (cell->getType() == HexCell::Type::BlockingBridge) {
                    cell->setType(HexCell::Type::Bridge);
                }
            }
        }
    }
}

HexChunk* HexGrid::findChunkForCoord(const game::HexCoord& coord) {
    for (auto& [id, chunkPtr] : _chunks) {
        if (chunkPtr->contains(coord)) {
            return chunkPtr.get();
        }
    }
    return nullptr;
}

} // namespace game
