#ifndef GAME_HEXCOORD_H
#define GAME_HEXCOORD_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <glm/glm.hpp>
#include <numbers>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

    constexpr HexCoord() : _q(0), _r(0) {}
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
    constexpr bool operator<(const HexCoord& other) const {
        if (_q != other._q) {
            return _q < other._q;
        }
        return _r < other._r;
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

    static constexpr std::array<Direction, 6> kDiagonals = {Direction::R0,   Direction::R60,
                                                            Direction::R120, Direction::R180,
                                                            Direction::R240, Direction::R300};

    static constexpr std::array<Direction, 6> kAxes = {Direction::R30,  Direction::R90,
                                                       Direction::R150, Direction::R210,
                                                       Direction::R270, Direction::R330};

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

    static std::optional<Direction> dir(HexCoord c) {
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
        for (std::size_t i = 0; i < Offsets.size(); i++) {
            if (Offsets.at(i).q() == c.q() && Offsets.at(i).r() == c.r()) {
                return static_cast<Direction>(i);
            }
        }
        return std::nullopt;
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

    [[nodiscard]] glm::vec3 toWorldPosition(float size, float spacing, float height) const {
        const auto qF = static_cast<float>(_q);
        const auto rF = static_cast<float>(_r);
        const float s = size + spacing;

        float x = s * (3.0F / 2.0F * qF);
        float z =
            s * ((std::numbers::sqrt3_v<float> / 2.0F * qF) + (std::numbers::sqrt3_v<float> * rF));

        return {z, height, x};
    }

    [[nodiscard]] HexCoord opposite() const {
        return {-_q, -_r};
    }

    [[nodiscard]] static HexCoord::Direction opposite(HexCoord::Direction d) {
        // R0 -> R330, @30deg step, start -> Pointing RIGHT
        static constexpr std::array<HexCoord::Direction, 12> Opposites = {
            HexCoord::Direction::R180, HexCoord::Direction::R210, HexCoord::Direction::R240,
            HexCoord::Direction::R270, HexCoord::Direction::R300, HexCoord::Direction::R330,
            HexCoord::Direction::R0,   HexCoord::Direction::R30,  HexCoord::Direction::R60,
            HexCoord::Direction::R90,  HexCoord::Direction::R120, HexCoord::Direction::R150};
        return Opposites.at(static_cast<std::size_t>(d));
    }

    [[nodiscard]] static std::vector<HexCoord> getNeighbors(HexCoord coord) {
        std::vector<HexCoord> neighbors;
        neighbors.reserve(6);
        for (int i = 0; i < 6; ++i) {
            neighbors.push_back(coord + dir(static_cast<Direction>(1 + (i * 2))));
        }
        return neighbors;
    }

    [[nodiscard]] static HexCoord getLinePoint(const HexCoord& a, const HexCoord& b, float t) {
        float fQ = static_cast<float>(a.q()) + (static_cast<float>(b.q() - a.q()) * t);
        float fR = static_cast<float>(a.r()) + (static_cast<float>(b.r() - a.r()) * t);
        float fS = -fQ - fR;

        float q = std::round(fQ);
        float r = std::round(fR);
        float s = std::round(fS);

        float qDiff = std::abs(q - fQ);
        float rDiff = std::abs(r - fR);
        float sDiff = std::abs(s - fS);

        if (qDiff > rDiff && qDiff > sDiff) {
            q = -r - s;
        } else if (rDiff > sDiff) {
            r = -q - s;
        }

        return {static_cast<int>(q), static_cast<int>(r)};
    }

    [[nodiscard]] static std::vector<HexCoord> hexesOnLine(const HexCoord& a, const HexCoord& b) {
        int n = distance(a, b);
        std::vector<HexCoord> results;
        results.reserve(n + 1);

        if (n == 0) {
            results.emplace_back(a);
            return results;
        }

        constexpr double kEpsilonQ = 1e-6;
        constexpr double kEpsilonR = 2e-6;
        constexpr double kEpsilonS = -3e-6;

        const double aQ = static_cast<double>(a.q()) + kEpsilonQ;
        const double aR = static_cast<double>(a.r()) + kEpsilonR;
        const double aS = static_cast<double>(a.s()) + kEpsilonS;

        const double bQ = static_cast<double>(b.q()) + kEpsilonQ;
        const double bR = static_cast<double>(b.r()) + kEpsilonR;
        const double bS = static_cast<double>(b.s()) + kEpsilonS;

        for (int i = 0; i <= n; i++) {
            const double t = static_cast<double>(i) / static_cast<double>(n);

            const double fQ = aQ + ((bQ - aQ) * t);
            const double fR = aR + ((bR - aR) * t);
            const double fS = aS + ((bS - aS) * t);

            double q = std::round(fQ);
            double r = std::round(fR);
            double s = std::round(fS);

            const double qDiff = std::abs(q - fQ);
            const double rDiff = std::abs(r - fR);
            const double sDiff = std::abs(s - fS);

            if (qDiff > rDiff && qDiff > sDiff) {
                q = -r - s;
            } else if (rDiff > sDiff) {
                r = -q - s;
            } else {
                s = -q - r;
            }

            results.emplace_back(static_cast<int>(q), static_cast<int>(r));
        }

        return results;
    }

  private:
    int _q;
    int _r;
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
            seed ^= static_cast<size_t>(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };
        combine(h.q());
        combine(h.r());
        return seed;
    }
};
} // namespace std

#endif
