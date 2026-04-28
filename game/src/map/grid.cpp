#include "map/grid.h"

#include "utils/perlin.h"

#include <algorithm>
#include <chrono>
#include <ranges>
#include <spdlog/spdlog.h>
#include <unordered_set>
#include <utility>

namespace game {
Grid::Grid(std::mt19937& rng) : _rng(rng) {}

std::pair<HexCoord, HexCoord> Grid::closestPair(std::size_t idx1, std::size_t idx2) {
    const auto& chunk1 = _chunks.at(idx1);
    const auto& chunk2 = _chunks.at(idx2);
    std::pair<HexCoord, HexCoord> closest{chunk1.getCenter(), chunk2.getCenter()};
    int minDist = std::numeric_limits<int>::max();
    for (const auto& cell1 : chunk1.getHexes()) {
        for (const auto& cell2 : chunk2.getHexes()) {
            auto dist = game::HexCoord::distance(cell1.coord, cell2.coord);
            if (dist < minDist) {
                minDist = dist;
                closest = {cell1.coord, cell2.coord};
            }
        }
    }
    return closest;
}

std::optional<std::size_t> Grid::makeChunk(HexCoord center, const HexChunk::Config& config) {
    auto constrainedConfig = config;
    constrainedConfig.canPlace = [this](const HexCoord& coord) {
        return !std::ranges::any_of(_cells,
                                    [&coord](const GridCell& cell) { return cell.coord == coord; });
    };

    HexChunk chunk(center, constrainedConfig);

    if (chunk.getHexes().empty()) {
        return std::nullopt;
    }

    Perlin perlinHoles(2);
    auto results =
        chunk.getHexes() |
        std::views::filter([&perlinHoles, holeChance = config.holeChance](const GridCell& cell) {
            float noiseVal = perlinHoles.noise(static_cast<float>(cell.coord.q()) * 0.1F,
                                               static_cast<float>(cell.coord.r()) * 0.1F);
            return noiseVal < holeChance && cell.state == GridCell::State::Occupied;
        });

    auto coordsView =
        results | std::views::transform([](const GridCell& cell) { return cell.coord; });
    chunk.remove(std::vector<HexCoord>{coordsView.begin(), coordsView.end()});

    if (chunk.getHexes().empty()) {
        return std::nullopt;
    }

    std::vector<GridCell> temp;
    temp.reserve(chunk.getHexes().size());
    for (const auto& cell : chunk.getHexes()) {
        if (cell.state != GridCell::State::Occupied) {
            continue;
        }
        temp.push_back(cell);
    }

    std::vector<std::pair<HexCoord, GridCell::OnHex>> assignedOnHex;
    if (temp.size() >= 3) {
        std::shuffle(temp.begin(), temp.end(), _rng);
        assignedOnHex.emplace_back(temp.at(0).coord, GridCell::OnHex::Enemy);
        assignedOnHex.emplace_back(temp.at(1).coord, GridCell::OnHex::Enemy);
        assignedOnHex.emplace_back(temp.at(2).coord, GridCell::OnHex::Resource);

        for (const auto& [coord, onHex] : assignedOnHex) {
            chunk.setOnHex(coord, onHex, boost::uuids::uuid{});
        }
    }

    for (const auto& cell : chunk.getHexes()) {
        auto it = _cells.find({.coord = cell.coord});
        if (it != _cells.end()) {
            _cells.erase(it);
        }
        _cells.insert(cell);
    }

    _chunks.push_back(std::move(chunk));
    return _chunks.size() - 1;
}

std::optional<std::size_t> Grid::makeChunk(std::size_t parentChunkIndex, HexCoord::Direction dir,
                                           const HexChunk::Config& config) {
    if (parentChunkIndex >= _chunks.size()) {
        spdlog::info("Invalid parent chunk index: {}", parentChunkIndex);
        return std::nullopt;
    }

    const auto& parentChunk = _chunks.at(parentChunkIndex);
    const int dist = parentChunk.getConfig().steps + config.steps + 2; // Add 2 for border
    const HexCoord nextCenter = parentChunk.getCenter() + HexCoord::dir(dir) * dist;

    auto constrainedConfig = config;
    constrainedConfig.canPlace = [this](const HexCoord& coord) {
        return !std::ranges::any_of(_cells,
                                    [&coord](const GridCell& cell) { return cell.coord == coord; });
    };

    HexChunk chunk(nextCenter, constrainedConfig);
    const auto* chunkHexes = &chunk.getHexes();

    if (chunkHexes->empty()) {
        return std::nullopt;
    }

    Perlin perlinHoles(2);
    auto results =
        *chunkHexes |
        std::views::filter([&perlinHoles, holeChance = config.holeChance](const GridCell& cell) {
            float noiseVal = perlinHoles.noise(static_cast<float>(cell.coord.q()) * 0.1F,
                                               static_cast<float>(cell.coord.r()) * 0.1F);
            return noiseVal < holeChance && cell.state == GridCell::State::Occupied;
        });

    auto coordsView =
        results | std::views::transform([](const GridCell& cell) { return cell.coord; });
    chunk.remove(std::vector<HexCoord>{coordsView.begin(), coordsView.end()});

    if (chunkHexes->empty()) {
        return std::nullopt;
    }

    // Post-porcessing -> Move new chunk towards parent chunk until they are 1 hex apart
    // Connect the chunks with a single-hex brigde

    std::vector<GridCell> temp;
    temp.reserve(chunkHexes->size());
    for (const auto& cell : *chunkHexes) {
        if (cell.state != GridCell::State::Occupied) {
            continue;
        }
        temp.push_back(cell);
    }

    std::vector<std::pair<HexCoord, GridCell::OnHex>> assignedOnHex;
    if (temp.size() >= 3) {
        std::shuffle(temp.begin(), temp.end(), _rng);
        assignedOnHex.emplace_back(temp.at(0).coord, GridCell::OnHex::Enemy);
        assignedOnHex.emplace_back(temp.at(1).coord, GridCell::OnHex::Enemy);
        assignedOnHex.emplace_back(temp.at(2).coord, GridCell::OnHex::Resource);

        for (const auto& [coord, onHex] : assignedOnHex) {
            chunk.setOnHex(coord, onHex, boost::uuids::uuid{});
        }
    }

    auto dirToParent = HexCoord::dir(HexCoord::dir(dir).opposite());
    if (dirToParent.has_value()) {
        chunk.setDirToParent(dirToParent.value());
    }

    while (parentChunk.intersection(chunk, true).empty()) {
        chunk.shift(dirToParent.value());
    }
    chunk.shift(dir);
    chunk.shift(dir);

    for (const auto& cell : chunk.getHexes()) {
        auto it = _cells.find({.coord = cell.coord});
        if (it != _cells.end()) {
            _cells.erase(it);
        }
        _cells.insert(cell);
    }

    _chunks.push_back(std::move(chunk));

    auto closest = closestPair(parentChunkIndex, _chunks.size() - 1);
    auto bridgeCoord = HexCoord::hexesOnLine(closest.first, closest.second);
    if (!bridgeCoord.empty()) {
        for (const auto& coord : bridgeCoord) {
            auto it = _cells.find({.coord = coord});
            if (it == _cells.end()) {
                _cells.insert({.coord = coord, .state = GridCell::State::Occupied});
                continue;
            }
            if (it->state == GridCell::State::Occupied) {
                continue;
            }
            _cells.erase(it);
            _cells.insert({.coord = coord, .state = GridCell::State::Occupied});
        }
    }

    return _chunks.size() - 1;
}
} // namespace game
