#include "map/HexGrid.h"

#include "boost/uuid/detail/nil_uuid.hpp"
#include "boost/uuid/uuid.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <utility>
#include <vector>

namespace game {
HexGrid::HexGrid(std::mt19937& rng) : _rng(rng) {}

namespace {
bool isBlocked(const HexChunk::HexCellPtr& cell) {
    return cell != nullptr && cell->getGenState() == HexCell::GenState::Blocked;
}

bool isNonBlocked(const HexChunk::HexCellPtr& cell) {
    return cell != nullptr && cell->getGenState() != HexCell::GenState::Blocked;
}

bool isOccupied(const HexChunk::HexCellPtr& cell) {
    return cell != nullptr;
}

bool touchesOtherChunk(
    const HexCoord& coord,
    const std::unordered_map<boost::uuids::uuid, std::unique_ptr<HexChunk>>& chunks,
    const boost::uuids::uuid& chunkToSkip) {
    for (const auto& [otherChunkId, otherChunk] : chunks) {
        if (otherChunkId == chunkToSkip) {
            continue;
        }

        if (isNonBlocked(otherChunk->getCell(coord))) {
            return true;
        }

        for (const auto& neighbor : HexCoord::getNeighbors(coord)) {
            if (isNonBlocked(otherChunk->getCell(neighbor))) {
                return true;
            }
        }
    }

    return false;
}

std::unique_ptr<HexChunk>
createChunkInstance(const HexChunk::Config& config,
                    const std::unordered_map<boost::uuids::uuid, std::unique_ptr<HexChunk>>& chunks,
                    bool hasParent, const boost::uuids::uuid& parentChunkId) {
    if (hasParent && !chunks.empty()) {
        auto parentIt = chunks.find(parentChunkId);
        if (parentIt == chunks.end()) {
            return nullptr;
        }
        return std::make_unique<HexChunk>(config, parentIt->second.get());
    }

    return std::make_unique<HexChunk>(config);
}

void removeOverlappingBlockedContacts(
    HexChunk& chunk,
    const std::unordered_map<boost::uuids::uuid, std::unique_ptr<HexChunk>>& chunks) {
    std::vector<HexCoord> toRemove;
    for (const auto& entry : chunk.getHexes()) {
        const auto& coord = entry.first;
        const auto& cell = entry.second;
        bool overlapsAnyCell = false;
        bool overlapsBlocked = false;
        for (const auto& existingChunkEntry : chunks) {
            auto existingCell = existingChunkEntry.second->getCell(coord);
            if (isOccupied(existingCell)) {
                overlapsAnyCell = true;
            }
            if (isBlocked(existingCell)) {
                overlapsBlocked = true;
            }
        }

        if (overlapsAnyCell) {
            toRemove.push_back(coord);
            continue;
        }

        if (!isNonBlocked(cell)) {
            continue;
        }

        if (overlapsBlocked && touchesOtherChunk(coord, chunks, boost::uuids::nil_uuid())) {
            toRemove.push_back(coord);
        }
    }

    if (!toRemove.empty()) {
        chunk.remove(toRemove);
    }
}

void connectParentAndChild(
    std::unordered_map<boost::uuids::uuid, std::unique_ptr<HexChunk>>& chunks,
    const boost::uuids::uuid& parentChunkId, const boost::uuids::uuid& chunkId,
    const std::pair<HexCoord, HexCoord>& closest) {
    auto hexes = HexCoord::hexesOnLine(closest.first, closest.second);

    for (const auto& hex : hexes) {
        bool inParent = chunks[parentChunkId]->contains(hex);
        bool inChild = chunks[chunkId]->contains(hex);

        if (inParent) {
            chunks[parentChunkId]->getCell(hex)->setGenState(HexCell::GenState::Protected);
            chunks[parentChunkId]->getCell(hex)->setType(HexCell::Type::Bridge);
        }

        if (inChild) {
            chunks[chunkId]->getCell(hex)->setGenState(HexCell::GenState::Protected);
            chunks[chunkId]->getCell(hex)->setType(HexCell::Type::Bridge);
        }

        if (!inParent && !inChild) {
            chunks[chunkId]->assignCell(std::make_shared<HexCell>(
                hex, HexCell::State::Empty, HexCell::Type::Bridge, HexCell::GenState::Protected));
        }
    }

    chunks[parentChunkId]->protectPathToOrigin(closest.first);
    chunks[chunkId]->protectPathToOrigin(closest.second);
}

void pruneNonBridgeContacts(
    const boost::uuids::uuid& chunkId,
    std::unordered_map<boost::uuids::uuid, std::unique_ptr<HexChunk>>& chunks) {
    auto& chunk = chunks.at(chunkId);
    std::vector<HexCoord> toRemove;
    for (const auto& [coord, cell] : chunk->getHexes()) {
        if (cell->getGenState() == HexCell::GenState::Blocked ||
            cell->getType() == HexCell::Type::Bridge) {
            continue;
        }

        if (touchesOtherChunk(coord, chunks, chunkId)) {
            toRemove.push_back(coord);
        }
    }

    if (!toRemove.empty()) {
        chunk->remove(toRemove);
    }
}
} // namespace

std::pair<HexCoord, HexCoord> HexGrid::closestPair(HexChunk* chunk1, HexChunk* chunk2) {
    std::pair<HexCoord, HexCoord> closest{chunk1->getHexes().begin()->first,
                                          chunk2->getHexes().begin()->first};
    bool found = false;
    int minDist = std::numeric_limits<int>::max();
    for (const auto& [coord1, cell1] : chunk1->getHexes()) {
        if (isBlocked(cell1)) {
            continue;
        }
        for (const auto& [coord2, cell2] : chunk2->getHexes()) {
            if (isBlocked(cell2)) {
                continue;
            }
            auto dist = game::HexCoord::distance(coord1, coord2);
            if (dist < minDist) {
                minDist = dist;
                closest = {coord1, coord2};
                found = true;
            }
        }
    }

    if (!found) {
        closest = {chunk1->getHexes().begin()->first, chunk2->getHexes().begin()->first};
    }

    return closest;
}

boost::uuids::uuid HexGrid::makeChunk(const HexChunk::Config& config) {
    auto parentChunkId = config.parentChunkId;

    bool hasParent = parentChunkId != boost::uuids::nil_uuid();

    if (!hasParent && !_chunks.empty()) {
        return boost::uuids::nil_uuid();
    }

    auto chunk = createChunkInstance(config, _chunks, hasParent, parentChunkId);
    if (chunk == nullptr) {
        return boost::uuids::nil_uuid();
    }

    if (chunk->getHexes().empty()) {
        return boost::uuids::nil_uuid();
    }

    removeOverlappingBlockedContacts(*chunk, _chunks);

    if (chunk->getHexes().empty()) {
        return boost::uuids::nil_uuid();
    }

    auto chunkId = chunk->getId();
    _chunks.insert({chunk->getId(), std::move(chunk)});

    if (hasParent) {
        auto closest = closestPair(_chunks[parentChunkId].get(), _chunks[chunkId].get());
        connectParentAndChild(_chunks, parentChunkId, chunkId, closest);

        pruneNonBridgeContacts(chunkId, _chunks);
    }

    return chunkId;
}

HexGrid::HexCellPtr HexGrid::getCell(const HexCoord& coord) const {
    for (const auto& [chunkId, chunk] : _chunks) {
        auto cell = chunk->getCell(coord);
        if (cell != nullptr) {
            return cell;
        }
    }

    return nullptr;
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
} // namespace game
