#ifndef GAME_GRIDCELL_H
#define GAME_GRIDCELL_H

#include "map/HexCoord.h"

#include <boost/uuid.hpp>
#include <cstdint>
#include <tuple>

namespace game {

struct GridCell {
    enum class State : uint8_t { Empty, Occupied, Blocked };
    enum class OnHex : uint8_t { None, Enemy, Player, Resource };

    HexCoord coord;
    State state{State::Empty};
    std::pair<boost::uuids::uuid, OnHex> onHex{boost::uuids::nil_uuid(), OnHex::None};

    bool operator<(const GridCell& other) const {
        return std::make_tuple(coord.q(), coord.r()) <
               std::make_tuple(other.coord.q(), other.coord.r());
    }

    bool operator==(const GridCell& other) const {
        return coord == other.coord && state == other.state;
    }
};
} // namespace game
namespace std {
template <> struct hash<game::GridCell> {
    size_t operator()(const game::GridCell& h) const noexcept {
        size_t seed = 0;
        auto combine = [&](size_t val) { seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2); };
        combine(hash<game::HexCoord>{}(h.coord));
        combine(hash<uint8_t>{}(static_cast<uint8_t>(h.state)));
        return seed;
    }
};
} // namespace std

#endif // GAME_GRIDCELL_H
