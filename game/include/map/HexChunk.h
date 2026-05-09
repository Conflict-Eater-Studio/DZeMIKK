#ifndef GAME_HEXCHUNK_H
#define GAME_HEXCHUNK_H

#include "boost/uuid/detail/nil_uuid.hpp"
#include "map/HexCell.h"

#include <functional>
#include <optional>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace game {
class Entity;
class HexChunk {
  public:
    struct Config {
        int steps{0};
        float holeChance{0.1F};
        std::function<float(int)> generator;
    };

    HexChunk(Config config, HexCoord origin);

    void setDirToParent(HexCoord::Direction dir);

    [[nodiscard]] const std::unordered_map<HexCoord, HexCell>& getHexes() const;
    std::unordered_map<HexCoord, HexCell>& getHexes();
    [[nodiscard]] const Config& getConfig() const;
    [[nodiscard]] const boost::uuids::uuid& getId() const;
    [[nodiscard]] HexCoord getOrigin() const;

    HexCell* updateAt(const HexCoord& coord);

    void remove(const std::vector<HexCoord>& hexes);
    [[nodiscard]] std::vector<HexCell> intersection(const HexChunk& other,
                                                    bool withBlocked = false) const;
    void shift(HexCoord::Direction dir, int times = 1);
    void assignCell(HexCell cell);
    bool setEntity(const HexCoord& coord, HexCell::Type entityType, Entity* entity = nullptr);

  private:
    boost::uuids::uuid _id{boost::uuids::nil_uuid()};
    std::unordered_map<HexCoord, HexCell> generateHexes();

    std::unordered_map<HexCoord, HexCell> _hexes;
    HexCoord _origin;
    Config _config{};
    std::optional<HexCoord::Direction> _dirToParent{std::nullopt};

    std::mt19937 _rng{std::random_device{}()};
    std::uniform_real_distribution<float> _chanceDist{0.0F, 1.0F};
};
} // namespace game

#endif // GAME_HEXCHUNK_H
