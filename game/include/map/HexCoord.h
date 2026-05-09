#ifndef GAME_HEXCOORD_H
#define GAME_HEXCOORD_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <numbers>
#include <optional>
#include <queue>
#include <unordered_map>
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

    [[nodiscard]] static std::vector<HexCoord> getNeighbors(HexCoord coord) {
        std::vector<HexCoord> neighbors;
        neighbors.reserve(6);
        for (int i = 0; i < 6; ++i) {
            neighbors.push_back(coord + dir(static_cast<Direction>(i * 2)));
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
        if (a == b) {
            return {a};
        }

        auto keyFor = [](const HexCoord& coord) {
            return (static_cast<std::int64_t>(coord.q()) << 32) |
                   static_cast<std::uint32_t>(coord.r());
        };

        auto coordFromKey = [](std::int64_t key) {
            int q = static_cast<int>(key >> 32);
            int r = static_cast<int>(static_cast<std::uint32_t>(key));
            return HexCoord{q, r};
        };

        const auto startKey = keyFor(a);
        const auto targetKey = keyFor(b);

        std::queue<HexCoord> frontier;
        std::unordered_map<std::int64_t, std::int64_t> parent;

        frontier.push(a);
        parent.emplace(startKey, startKey);

        bool found = false;
        while (!frontier.empty() && !found) {
            const HexCoord current = frontier.front();
            frontier.pop();
            const auto currentKey = keyFor(current);

            for (const auto& neighbor : getNeighbors(current)) {
                const auto neighborKey = keyFor(neighbor);
                if (parent.contains(neighborKey)) {
                    continue;
                }

                parent.emplace(neighborKey, currentKey);

                if (neighborKey == targetKey) {
                    found = true;
                    break;
                }

                frontier.push(neighbor);
            }
        }

        if (!found) {
            return {a, b};
        }

        std::vector<HexCoord> path;
        for (auto currentKey = targetKey; currentKey != startKey;
             currentKey = parent.at(currentKey)) {
            path.push_back(coordFromKey(currentKey));
        }
        path.push_back(a);
        std::ranges::reverse(path);

        return path;
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
            seed ^= static_cast<size_t>(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };
        combine(h.q());
        combine(h.r());
        return seed;
    }
};
} // namespace std

#endif
