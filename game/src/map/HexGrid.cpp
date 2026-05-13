#include "map/HexGrid.h"

#include "boost/uuid/detail/nil_uuid.hpp"
#include "boost/uuid/uuid.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <utility>

namespace game {
namespace {
using ChunkMap = std::unordered_map<boost::uuids::uuid, std::unique_ptr<HexChunk>>;

void setBridgeCell(HexChunk& chunk, const HexCoord& coord) {
    auto it = chunk.getHexes().find(coord);
    if (it != chunk.getHexes().end()) {
        auto& cell = it->second;
        cell->setType(HexCell::Type::Bridge);
        cell->setState(HexCell::State::Empty);
        cell->setGenState(HexCell::GenState::Protected);
        return;
    }

    chunk.assignCell(std::make_shared<HexCell>(coord, HexCell::State::Empty, HexCell::Type::Bridge,
                                               HexCell::GenState::Protected));
}

void buildBridge(ChunkMap& chunks, const boost::uuids::uuid& chunkId,
                 const boost::uuids::uuid& parentChunkId, HexCoord::Direction dirToParent,
                 HexCoord::Direction dirFromParent) {
    auto h1 = chunks[chunkId]->getFurthestEdgeHexes().at(dirToParent);
    auto h2 = chunks[parentChunkId]->getFurthestEdgeHexes().at(dirFromParent);
    auto bridge = HexCoord::hexesOnLine(h1->getCoord(), h2->getCoord());

    for (const auto& coord : bridge) {
        if (chunks[chunkId]->contains(coord)) {
            setBridgeCell(*chunks[chunkId], coord);
            continue;
        }

        if (chunks[parentChunkId]->contains(coord)) {
            setBridgeCell(*chunks[parentChunkId], coord);
            continue;
        }

        setBridgeCell(*chunks[chunkId], coord);
    }
}
} // namespace

HexGrid::HexGrid(std::mt19937& rng) : _rng(rng) {}

std::pair<HexCoord, HexCoord> HexGrid::closestPair(HexChunk* chunk1, HexChunk* chunk2) {
    std::pair<HexCoord, HexCoord> closest{chunk1->getHexes().begin()->first,
                                          chunk2->getHexes().begin()->first};
    int minDist = std::numeric_limits<int>::max();
    for (const auto& [coord1, cell1] : chunk1->getHexes()) {
        for (const auto& [coord2, cell2] : chunk2->getHexes()) {
            auto dist = game::HexCoord::distance(coord1, coord2);
            if (dist < minDist) {
                minDist = dist;
                closest = {coord1, coord2};
            }
        }
    }
    return closest;
}

boost::uuids::uuid HexGrid::makeChunk(const HexChunk::Config& config) {
    auto parentChunkId = config.parentChunkId;
    auto dirToParent = HexCoord::opposite(config.dirFromParent);
    auto dirFromParent = config.dirFromParent;

    bool hasParent = parentChunkId != boost::uuids::nil_uuid();

    if (!hasParent && !_chunks.empty()) {
        return boost::uuids::nil_uuid();
    }

    std::unique_ptr<HexChunk> chunk = nullptr;
    if (hasParent && !_chunks.empty()) {
        std::unique_ptr<HexChunk>& parentChunk = _chunks[parentChunkId];
        chunk = std::make_unique<HexChunk>(config, parentChunk.get());
    } else {
        chunk = std::make_unique<HexChunk>(config);
    }

    auto chunkHexes = chunk->getHexes();

    if (chunkHexes.empty()) {
        return boost::uuids::nil_uuid();
    }

    // if (hasParent) {
    //     const auto& parentChunk = _chunks[parentChunkId];
    //     while (parentChunk->intersection(*chunk, true).empty()) {
    //         chunk->shift(dirToParent, 1);
    //     }
    //     chunk->shift(dirFromParent, 2);
    // }

    auto chunkId = chunk->getId();
    _chunks.insert({chunk->getId(), std::move(chunk)});

    if (hasParent) {
        buildBridge(_chunks, chunkId, parentChunkId, dirToParent, dirFromParent);
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
