#ifndef GAME_HEXPATTERN_H
#define GAME_HEXPATTERN_H

#include "map/HexCoord.h"

#include <vector>
namespace game {
class HexPattern {
  public:
    enum class Type : uint8_t { ATK, DEF, HEAL, BONUSHEX };
    enum class Rotation : uint8_t { Clockwise, CounterClockwise };

    HexPattern(std::vector<HexCoord> hexes, Type type, float effectStrength = 1.0F);

    [[nodiscard]] const std::vector<HexCoord>& getHexes() const;
    [[nodiscard]] const std::vector<HexCoord>& rotate(Rotation rotation = Rotation::Clockwise);

  private:
    std::vector<HexCoord> _hexes;
    Type _type{Type::ATK};
    float _effectStrength{1.0F};
};
} // namespace game

#endif // GAME_HEXPATTERN_H
