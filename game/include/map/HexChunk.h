#ifndef GAME_HEXCHUNK_H
#define GAME_HEXCHUNK_H

#include "map/gridcell.h"

#include <functional>
#include <optional>
#include <random>
#include <set>
#include <unordered_set>
#include <vector>

namespace game {
class HexChunk {
  public:
    struct Config {
        int steps{0};
        float holeChance{0.1F};
        std::function<float(int)> generator;
        std::function<bool(const HexCoord&)> canPlace;
    };

    HexChunk() = default;
    HexChunk(HexCoord center, Config config);

    void setDirToParent(HexCoord::Direction dir);

    [[nodiscard]] const std::unordered_set<GridCell>& getHexes() const;
    [[nodiscard]] HexCoord getCenter() const;
    [[nodiscard]] const Config& getConfig() const;

    void remove(const std::vector<HexCoord>& hexes);
    [[nodiscard]] std::vector<GridCell> intersection(const HexChunk& other,
                                                     bool withBlocked = false) const;
    void shift(HexCoord::Direction dir);
    void markChunk();
    void assignCell(GridCell cell);

  private:
    std::unordered_set<GridCell> generateHexes();

    std::unordered_set<GridCell> _hexes;
    HexCoord _center{0, 0};
    Config _config{};
    std::optional<HexCoord::Direction> _dirToParent{std::nullopt};

    std::mt19937 _rng{std::random_device{}()};
    std::uniform_real_distribution<float> _chanceDist{0.0F, 1.0F};
};
} // namespace game

#endif // GAME_HEXCHUNK_H
