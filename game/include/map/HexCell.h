#ifndef GAME_GRIDCELL_H
#define GAME_GRIDCELL_H

#include "map/HexCoord.h"

#include <boost/uuid.hpp>
#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

namespace game {

class Entity;
class HexCell {

  public:
    enum class State : uint8_t { Prop, Item, Player, Enemy, Empty };
    enum class Type : uint8_t { Normal, PlayerBattleHex, EnemyBattleHex, Bridge, Blocked };

    HexCell() : _coord(0, 0) {}
    HexCell(HexCoord coord, State state, Type type, std::shared_ptr<Entity> entity = nullptr)
        : _coord(coord), _state(state), _type(type), _entity(std::move(entity)) {}

    [[nodiscard]] const HexCoord& getCoord() const {
        return _coord;
    }
    [[nodiscard]] HexCoord& getCoord() {
        return _coord;
    }
    [[nodiscard]] State getState() const {
        return _state;
    }
    [[nodiscard]] Type getType() const {
        return _type;
    }
    [[nodiscard]] std::shared_ptr<Entity> getEntity() const {
        return _entity;
    }

    void setCoord(const HexCoord& coord) {
        _coord = coord;
    }
    void setState(State state) {
        _state = state;
    }
    void setType(Type type) {
        _type = type;
    }
    void setEntity(Entity* entity) {
        _entity = std::shared_ptr<Entity>(entity, [](Entity*) {});
    }

    bool operator<(const HexCell& other) const {
        return std::make_tuple(_coord.q(), _coord.r()) <
               std::make_tuple(other._coord.q(), other._coord.r());
    }

    bool operator==(const HexCell& other) const {
        return _coord == other._coord && _state == other._state;
    }

  private:
    HexCoord _coord{0, 0};
    State _state{State::Empty};
    Type _type{Type::Normal};

    std::shared_ptr<Entity> _entity = nullptr;
};
} // namespace game
namespace std {
template <> struct hash<game::HexCell> {
    size_t operator()(const game::HexCell& h) const noexcept {
        size_t seed = 0;
        auto combine = [&](size_t val) { seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2); };
        combine(hash<game::HexCoord>{}(h.getCoord()));
        combine(hash<uint8_t>{}(static_cast<uint8_t>(h.getState())));
        return seed;
    }
};
} // namespace std

#endif // GAME_GRIDCELL_H
