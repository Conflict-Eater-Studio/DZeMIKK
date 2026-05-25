#ifndef PERLIN_HPP
#define PERLIN_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <numeric>
#include <random>

namespace game {
class Perlin {
  public:
    explicit Perlin(unsigned int seed) {
        std::iota(_p.begin(), _p.begin() + 256, 0);

        std::default_random_engine engine(seed);
        std::shuffle(_p.begin(), _p.begin() + 256, engine);

        for (int i = 0; i < 256; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            _p[256 + i] = _p[i];
        }
    }

    [[nodiscard]] float noise(float x, float y) const {
        int xx = static_cast<int>(std::floor(x)) & 255;
        int yy = static_cast<int>(std::floor(y)) & 255;

        x -= std::floor(x);
        y -= std::floor(y);

        float u = fade(x);
        float v = fade(y);

        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
        int aa = _p[_p[xx] + yy];
        int ab = _p[_p[xx] + yy + 1];
        int ba = _p[_p[xx + 1] + yy];
        int bb = _p[_p[xx + 1] + yy + 1];
        // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)

        float rawNoise = lerp(v, lerp(u, grad(aa, x, y), grad(ba, x - 1, y)),
                              lerp(u, grad(ab, x, y - 1), grad(bb, x - 1, y - 1)));

        // NOTE: May not be perfectly normalized
        return (rawNoise + 0.707F) / 1.141F; // Normalize to roughly [0, 1]
    }

#ifdef DZEMIKK_DEV_TOOLS
    void savePerlinPPM(const std::string& filename, int width, int height, float scale) const {
        std::fstream ofs(filename, std::ios::binary | std::ios::out);
        if (!ofs) {
            return;
        }

        // PPM Header: P6 means binary, then width, height, and max color value (255)
        ofs << "P6\n" << width << " " << height << "\n255\n";

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Map pixel coordinates to noise coordinates
                float nx = static_cast<float>(x) * scale / static_cast<float>(width);
                float ny = static_cast<float>(y) * scale / static_cast<float>(height);

                // Get noise value and clamp it to [0, 1]
                float val = noise(nx, ny);
                val = std::clamp(val, 0.0F, 1.0F);

                // Convert to grayscale byte (0-255)
                auto color = static_cast<unsigned char>(val * 255.0F);

                // Write R, G, and B components
                ofs << color << color << color;
            }
        }
        ofs.close();
    }
#endif

  private:
    std::array<int, 512> _p{};

    static float fade(float t) {
        return t * t * t * ((t * ((t * 6.0F) - 15.0F)) + 10.0F);
    }

    static float lerp(float t, float a, float b) {
        return a + (t * (b - a));
    }

    static float grad(int hash, float x, float y) {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : (h == 12 || h == 14 ? x : 0);
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
};
} // namespace game
#endif
