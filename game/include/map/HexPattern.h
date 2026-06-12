#ifndef GAME_HEXPATTERN_H
#define GAME_HEXPATTERN_H

#include "map/HexCoord.h"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <vector>

namespace game {
class HexPattern {
  public:
    enum class Type : uint8_t { ATK, DEF, HEAL, BONUSHEX };
    enum class Rotation : uint8_t { Clockwise, CounterClockwise };

    HexPattern() : _hexes({{0, 0}}), _type(Type::ATK), _effectStrength(1.0F) {}

    HexPattern(std::vector<HexCoord> hexes, Type type, float effectStrength = 1.0F);
    ~HexPattern() = default;

    [[nodiscard]] const std::vector<HexCoord>& getHexes() const;
    [[nodiscard]] const std::vector<HexCoord>& rotate(Rotation rotation = Rotation::Clockwise);
    [[nodiscard]] const Type getType() const;

    bool operator==(const HexPattern& other) const;
    bool operator!=(const HexPattern& other) const;

    [[nodiscard]]
    float getEffectStrength() const {
        return _effectStrength;
    }

  private:
    std::vector<HexCoord> _hexes;
    Type _type{Type::ATK};
    float _effectStrength{1.0F};
};

// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& j, const HexPattern& pat) {
    j = nlohmann::json{{"hexes", pat.getHexes()},
                       {"effectStrenght", pat.getEffectStrength()},
                       {"type", pat.getType()}};
}

inline void from_json(const nlohmann::json& j, HexPattern& pat) {
    if (!j.contains("hexes") || !j.contains("effectStrenght") || !j.contains("type") ||
        !j["hexes"].is_array() || j["hexes"].empty() || !j["effectStrenght"].is_number_float() ||
        !j["type"].is_number_integer()) {
        throw std::runtime_error("Invalid JSON");
    }

    std::vector<HexCoord> hexes = j["hexes"].get<std::vector<HexCoord>>();
    pat = HexPattern(hexes, static_cast<HexPattern::Type>(j["type"].get<int>(),
                                                          j["effectStrenght"].get<float>()));
}
// NOLINTEND(readability-identifier-naming)

} // namespace game
#endif // GAME_HEXPATTERN_H
