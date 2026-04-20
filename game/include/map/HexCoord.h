#ifndef GAME_HEXCOORD_H
#define GAME_HEXCOORD_H

#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <numbers>

#ifdef DZEMIKK_DEV_TOOLS
#include <iosfwd>
#endif

namespace game {

class HexCoord {
  public:
    enum class Direction : std::uint8_t {
        R0,
        R30,
        R60,
        R90,
        R120,
        R150,
        R180,
        R210,
        R240,
        R270,
        R300,
        R330
    };

    constexpr HexCoord(int q, int r) : _q(q), _r(r) {}

    constexpr HexCoord operator+(const HexCoord& other) const {
        return {_q + other._q, _r + other._r};
    }
    constexpr HexCoord operator-(const HexCoord& other) const {
        return {_q - other._q, _r - other._r};
    }
    constexpr HexCoord operator*(int scalar) const {
        return {_q * scalar, _r * scalar};
    }

    constexpr bool operator==(const HexCoord& other) const {
        return _q == other._q && _r == other._r;
    }
    constexpr bool operator!=(const HexCoord& other) const {
        return !(*this == other);
    }

    HexCoord& operator+=(const HexCoord& other) {
        _q += other._q;
        _r += other._r;
        return *this;
    }
    HexCoord& operator-=(const HexCoord& other) {
        _q -= other._q;
        _r -= other._r;
        return *this;
    }
    HexCoord& operator*=(int scalar) {
        _q *= scalar;
        _r *= scalar;
        return *this;
    }

    static const HexCoord& dir(Direction d) {
        // R0 -> R330, @30deg step, start -> Pointing RIGHT
        static constexpr std::array<HexCoord, 12> Offsets = {{{2, -1},
                                                              {1, -1},
                                                              {1, -2},
                                                              {0, -1},
                                                              {-1, -1},
                                                              {-1, 0},
                                                              {-2, 1},
                                                              {-1, 1},
                                                              {-1, 2},
                                                              {0, 1},
                                                              {1, 1},
                                                              {1, 0}}};
        return Offsets.at(static_cast<std::size_t>(d));
    }

    [[nodiscard]] constexpr int q() const {
        return _q;
    }
    [[nodiscard]] constexpr int r() const {
        return _r;
    }
    [[nodiscard]] constexpr int s() const {
        return -_q - _r;
    }

    static constexpr int distance(const HexCoord& a, const HexCoord& b) {
        auto d = a - b;
        auto absQ = d.q() < 0 ? -d.q() : d.q();
        auto absR = d.r() < 0 ? -d.r() : d.r();
        auto absS = d.s() < 0 ? -d.s() : d.s();
        return (absQ + absR + absS) / 2;
    }

    [[nodiscard]] glm::vec3 toWorldPosition(float size, float spacing) const {
        const auto qF = static_cast<float>(_q);
        const auto rF = static_cast<float>(_r);
        const float s = size + spacing;

        float x = s * (3.0F / 2.0F * qF);
        float z =
            s * ((std::numbers::sqrt3_v<float> / 2.0F * qF) + (std::numbers::sqrt3_v<float> * rF));

        return {x, _height, z};
    }

    [[nodiscard]] HexCoord opposite() const {
        return {-_q, -_r};
    }

    void setHeight(float height) {
        _height = height;
    }
    [[nodiscard]] float getHeight() const {
        return _height;
    }

  private:
    int _q;
    int _r;
    float _height{0};
};

inline HexCoord operator*(int scalar, HexCoord::Direction d) {
    return HexCoord::dir(d) * scalar;
}

#ifdef DZEMIKK_DEV_TOOLS
}
#include <ostream>
namespace game {
inline std::ostream& operator<<(std::ostream& os, const HexCoord& coord) {
    return os << "HexCoord(q: " << coord.q() << ", r: " << coord.r() << ")";
}

inline std::ostream& operator<<(std::ostream& os, const HexCoord::Direction& dir) {
    static constexpr std::array<const char*, 12> DirNames = {
        "R0", "R30", "R60", "R90", "R120", "R150", "R180", "R210", "R240", "R270", "R300", "R330"};
    return os << DirNames.at(static_cast<std::size_t>(dir));
}
#endif
}

namespace std {
template <> struct hash<game::HexCoord> {
    size_t operator()(const game::HexCoord& h) const noexcept {
        size_t seed = 0;
        auto combine = [&](int val) {
            seed ^= std::hash<int>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };
        combine(h.q());
        combine(h.r());
        return seed;
    }
};
} // namespace std

#endif
