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

    enum class State : uint8_t { Prop, Item, Player, Enemy, Empty, Totem, TotemDialog, Path};
    enum class Type : uint8_t {
        Normal,
        PlayerBattleHex,
        EnemyBattleHex,
        Bridge,
        BlockingPattern,
        BlockingBridge
    };
    enum class GenState : uint8_t { Normal, Blocked, Protected };

    HexCell() : _coord(0, 0) {}
    HexCell(HexCoord coord, State state, Type type, GenState genState = GenState::Normal,
            Entity* entity = nullptr)
        : _coord(coord),
          _flags(pack(state, type, genState, false, false, false)),
          _entity(entity) {}

    [[nodiscard]] const HexCoord& getCoord() const {
        return _coord;
    }
    [[nodiscard]] HexCoord& getCoord() {
        return _coord;
    }
    [[nodiscard]] State getState() const {
        return static_cast<State>(_flags & 0xFF);
    }
    [[nodiscard]] Type getType() const {
        return static_cast<Type>((_flags >> 8) & 0xFF);
    }
    [[nodiscard]] GenState getGenState() const {
        return static_cast<GenState>((_flags >> 16) & 0xFF);
    }
    [[nodiscard]] Entity* getEntity() const {
        return _entity;
    }

    [[nodiscard]] float getHeight() const {
        return _height;
    }

    [[nodiscard]] bool isDirty() const {
        return (_flags >> 24) & 1;
    }
    [[nodiscard]] bool isCheckpoint() const {
        return (_flags >> 25) & 1;
    }
    [[nodiscard]] bool isCheckpointUsed() const {
        return (_flags >> 26) & 1;
    }

    void setState(State state) {
        _flags = (_flags & ~static_cast<uint32_t>(0xFF)) | static_cast<uint8_t>(state);
        _flags |= kDirty;
    }
    void setType(Type type) {
        _flags = (_flags & ~static_cast<uint32_t>(0xFF00)) |
                 (static_cast<uint32_t>(static_cast<uint8_t>(type)) << 8);
        _flags |= kDirty;
    }
    void setGenState(GenState genState) {
        _flags = (_flags & ~static_cast<uint32_t>(0xFF0000)) |
                 (static_cast<uint32_t>(static_cast<uint8_t>(genState)) << 16);
        _flags |= kDirty;
    }
    void setEntity(Entity* entity) {
        _entity = entity;
        _flags |= kDirty;
    }
    void setHeight(float height) {
        _height = height;
        _flags |= kDirty;
    }
    void setDirty(bool dirty) {
        if (dirty) {
            _flags |= kDirty;
        } else {
            _flags &= ~kDirty;
        }
    }
    void setCheckpoint(bool checkpoint) {
        if (checkpoint) {
            _flags |= kCheckpoint;
        } else {
            _flags &= ~kCheckpoint;
        }
        _flags |= kDirty;
    }
    void setCheckpointUsed(bool used) {
        if (used) {
            _flags |= kCheckpointUsed;
        } else {
            _flags &= ~kCheckpointUsed;
        }
        _flags |= kDirty;
    }

    void resetGenState() {
        _flags &= ~static_cast<uint32_t>(0xFF0000);
    }

    [[nodiscard]] uint32_t getFlags() const {
        return _flags;
    }

    bool operator<(const HexCell& other) const {
        return std::make_tuple(_coord.q(), _coord.r()) <
               std::make_tuple(other._coord.q(), other._coord.r());
    }

    bool operator==(const HexCell& other) const {
        return _coord == other._coord && getState() == other.getState();
    }

  private:
    static constexpr uint32_t kDirty = 1U << 24;
    static constexpr uint32_t kCheckpoint = 1U << 25;
    static constexpr uint32_t kCheckpointUsed = 1U << 26;

    static constexpr uint32_t pack(State s, Type t, GenState g, bool dirty, bool checkpoint,
                                   bool checkpointUsed) {
        return static_cast<uint32_t>(static_cast<uint8_t>(s)) |
               (static_cast<uint32_t>(static_cast<uint8_t>(t)) << 8) |
               (static_cast<uint32_t>(static_cast<uint8_t>(g)) << 16) | (dirty ? kDirty : 0) |
               (checkpoint ? kCheckpoint : 0) | (checkpointUsed ? kCheckpointUsed : 0);
    }

    void setCoord(const HexCoord& coord) {
        _coord = coord;
    }

    HexCoord _coord{0, 0};
    // 0-7: State, 8-15: Type, 16-23: GenState, 24: Dirty, 25: Checkpoint, 26: CheckpointUsed
    uint32_t _flags{0};
    Entity* _entity = nullptr;

    float _height{0.0F};
};

// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& j, const game::HexCell& cell) {
    j = nlohmann::json{{"coord", cell.getCoord()},
                       {"state", static_cast<uint8_t>(cell.getState())},
                       {"type", static_cast<uint8_t>(cell.getType())},
                       {"height", cell.getHeight()},
                       {"checkpoint", cell.isCheckpoint()},
                       {"checkpointUsed", cell.isCheckpointUsed()}};
}

inline void from_json(const nlohmann::json& j, game::HexCell& cell) {
    cell = game::HexCell(j.at("coord").get<game::HexCoord>(),
                         static_cast<game::HexCell::State>(j.at("state").get<uint8_t>()),
                         static_cast<game::HexCell::Type>(j.at("type").get<uint8_t>()),
                         game::HexCell::GenState::Normal);
    if (j.contains("height")) {
        cell.setHeight(j.at("height").get<float>());
    }
    if (j.contains("checkpoint")) {
        cell.setCheckpoint(j.at("checkpoint").get<bool>());
    }
    if (j.contains("checkpointUsed")) {
        cell.setCheckpointUsed(j.at("checkpointUsed").get<bool>());
    }
}
// NOLINTEND(readability-identifier-naming)

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
