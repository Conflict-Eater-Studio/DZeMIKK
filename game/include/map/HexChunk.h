#ifndef GAME_HEXCHUNK_H
#define GAME_HEXCHUNK_H

#include "boost/uuid/detail/nil_uuid.hpp"
#include "map/HexCell.h"

#include <functional>
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

namespace game {
class Entity;
class HexChunk {
  public:
    using HexCellPtr = std::shared_ptr<HexCell>;

    struct Config {
        boost::uuids::uuid parentChunkId{boost::uuids::nil_uuid()};
        int steps{0};
        float holeChance{0.1F};
        std::function<float(int)> generator;
        HexCoord::Direction dirFromParent{HexCoord::Direction::R180};
    };

    HexChunk(Config config, HexChunk* parent);
    HexChunk(Config config);

    [[nodiscard]] const std::unordered_map<HexCoord, HexCellPtr>& getHexes() const;
    std::unordered_map<HexCoord, HexCellPtr>& getHexes();
    [[nodiscard]] HexCellPtr getCell(const HexCoord& coord) const;
    [[nodiscard]] bool contains(const HexCoord& coord) const;
    HexCellPtr extractCell(const HexCoord& coord);
    bool insertCell(const HexCoord& coord, HexCellPtr cell);
    [[nodiscard]] const Config& getConfig() const;
    [[nodiscard]] const boost::uuids::uuid& getId() const;
    [[nodiscard]] HexCoord getOrigin() const;
    void protectPathToOrigin(const HexCoord& start);

    void remove(const std::vector<HexCoord>& hexes);
    [[nodiscard]] std::vector<HexCoord> intersection(const HexChunk& other,
                                                     bool withBlocked = false) const;
    void shift(HexCoord::Direction dir, int times = 1);
    void assignCell(HexCellPtr cell);
    bool setEntity(const HexCoord& coord, HexCell::Type entityType, Entity* entity = nullptr);

  private:
    boost::uuids::uuid _id{boost::uuids::nil_uuid()};
    void generateHexes();
    void generateHexCells();
    void fillBlockedHexes(int minQ, int maxQ, int minR, int maxR, int minS, int maxS);
    void blockHexesWithMaxNeighbours(int maxNeighbours);
    void unblockIsolatedHexes();

    HexChunk* _parent{nullptr};
    std::unordered_map<HexCoord, HexCellPtr> _hexes;
    HexCoord _origin{0, 0};
    Config _config{};

    std::mt19937 _rng{std::random_device{}()};
    std::uniform_real_distribution<float> _chanceDist{0.0F, 1.0F};
};
} // namespace game

#endif // GAME_HEXCHUNK_H
