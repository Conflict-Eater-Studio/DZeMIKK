#ifndef GAME_GRIDCELL_H
#define GAME_GRIDCELL_H

#include "map/HexCoord.h"

#include <boost/uuid.hpp>
#include <cstdint>
#include <tuple>
#include <utility>

namespace game {

class Entity;
class EnemyEntity;

class HexCell {

  public:
    friend class HexChunk;

    enum class State : uint8_t { Prop, Item, Player, Enemy, Empty };
    enum class Type : uint8_t { Normal, PlayerBattleHex, EnemyBattleHex, Bridge };
    enum class GenState : uint8_t { Normal, Blocked, Protected };

    HexCell() : _coord(0, 0) {}
    HexCell(HexCoord coord, State state, Type type, GenState genState = GenState::Normal,
            Entity* entity = nullptr)
        : _coord(coord), _state(state), _type(type), _genState(genState), _entity(entity) {}

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
    [[nodiscard]] GenState getGenState() const {
        return _genState;
    }
    [[nodiscard]] Entity* getEntity() const {
        return _entity;
    }

    [[nodiscard]] bool isDirty() const {
        return _dirty;
    }
    void setState(State state) {
        _state = state;
        _dirty = true;
    }
    void setType(Type type) {
        _type = type;
        _dirty = true;
    }
    void setGenState(GenState genState) {
        _genState = genState;
        _dirty = true;
    }
    void setEntity(Entity* entity) {
        _entity = entity;
        _dirty = true;
    }
    void setDirty(bool dirty) {
        _dirty = dirty;
    }

    bool operator<(const HexCell& other) const {
        return std::make_tuple(_coord.q(), _coord.r()) <
               std::make_tuple(other._coord.q(), other._coord.r());
    }

    bool operator==(const HexCell& other) const {
        return _coord == other._coord && _state == other._state;
    }

  private:

    void setCoord(const HexCoord& coord) {
        _coord = coord;
    }

    HexCoord _coord{0, 0};
    State _state{State::Empty};
    Type _type{Type::Normal};
    GenState _genState{GenState::Normal};

    Entity* _entity = nullptr;

    bool _dirty = false;
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
