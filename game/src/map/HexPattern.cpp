#include "map/HexPattern.h"

#include <algorithm>

namespace game {
HexPattern::HexPattern(std::vector<HexCoord> hexes, Type type, float effectStrength)
    : _type(type), _effectStrength(effectStrength) {
    if (hexes.empty()) {
        throw std::invalid_argument("HexPattern cannot be empty");
    }

    if (std::ranges::find_if(hexes, [](const HexCoord& h) { return h == HexCoord{0, 0}; }) ==
        hexes.end()) {
        _hexes.emplace_back(0, 0);
    }

    _hexes.insert(_hexes.end(), hexes.begin(), hexes.end());
}

const std::vector<HexCoord>& HexPattern::getHexes() const {
    return _hexes;
}

const std::vector<HexCoord>& HexPattern::rotate(Rotation rotation) {
    std::vector<HexCoord> rotatedHexes;
    rotatedHexes.reserve(_hexes.size());
    for (const auto& hex : _hexes) {
        if (rotation == Rotation::Clockwise) {
            rotatedHexes.emplace_back(-hex.r(), hex.q() + hex.r());
        } else {
            rotatedHexes.emplace_back(hex.q() + hex.r(), -hex.q());
        }
    }
    _hexes = std::move(rotatedHexes);
    return _hexes;
}
const game::HexPattern::Type HexPattern::getType() const {
    return _type;
}
} // namespace game
