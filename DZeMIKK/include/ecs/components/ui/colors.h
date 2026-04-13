#ifndef DZEMIKK_COLOR_H
#define DZEMIKK_COLOR_H

#include <glm/glm.hpp>
#include <stdexcept>
#include <string>

namespace dzemikk {
class Colors {
  public:
    static constexpr glm::vec4 White{1.0F, 1.0F, 1.0F, 1.0F};
    static constexpr glm::vec4 Black{0.0F, 0.0F, 0.0F, 1.0F};
    static constexpr glm::vec4 Red{1.0F, 0.0F, 0.0F, 1.0F};
    static constexpr glm::vec4 Green{0.0F, 1.0F, 0.0F, 1.0F};
    static constexpr glm::vec4 Blue{0.0F, 0.0F, 1.0F, 1.0F};

    static constexpr glm::vec4 Yellow{1.0F, 1.0F, 0.0F, 1.0F};
    static constexpr glm::vec4 Cyan{0.0F, 1.0F, 1.0F, 1.0F};
    static constexpr glm::vec4 Magenta{1.0F, 0.0F, 1.0F, 1.0F};

    static constexpr glm::vec4 Gray{0.5f, 0.5f, 0.5f, 1.0F};
    static constexpr glm::vec4 LightGray{0.8f, 0.8f, 0.8f, 1.0F};
    static constexpr glm::vec4 DarkGray{0.2f, 0.2f, 0.2f, 1.0F};

    static constexpr glm::vec4 Transparent{0.0F, 0.0F, 0.0F, 0.0F};

    static glm::vec4 fromRGBA(float r, float g, float b, float a) {
        return {r, g, b, a};
    }

    static glm::vec4 fromRGB(float r, float g, float b) {
        return {r, g, b, 1.0F};
    }

    static glm::vec4 fromHex(const std::string& hex) {
        if (hex.size() != 7 || hex[0] != '#') {
            throw std::runtime_error("Invalid hex color format. Expected format: #RRGGBB");
        }
        unsigned int r = std::stoul(hex.substr(1, 2), nullptr, 16);
        unsigned int g = std::stoul(hex.substr(3, 2), nullptr, 16);
        unsigned int b = std::stoul(hex.substr(5, 2), nullptr, 16);
        return {(float)r / 255.0F, (float)g / 255.0F, (float)b / 255.0F, 1.0F};
    }

    static glm::vec4 fromHexA(const std::string& hex) {
        if (hex.size() != 9 || hex[0] != '#') {
            throw std::runtime_error("Invalid hex color format. Expected format: #RRGGBBAA");
        }
        unsigned int r = std::stoul(hex.substr(1, 2), nullptr, 16);
        unsigned int g = std::stoul(hex.substr(3, 2), nullptr, 16);
        unsigned int b = std::stoul(hex.substr(5, 2), nullptr, 16);
        unsigned int a = std::stoul(hex.substr(7, 2), nullptr, 16);
        return {(float)r / 255.0F, (float)g / 255.0F, (float)b / 255.0F, (float)a / 255.0F};
    }

    static glm::vec4 fromHSL(float h, float s, float l) {
        float c = (1.0F - glm::abs((2.0F * l) - 1.0F)) * s;
        float x = c * (1.0F - glm::abs(glm::mod(h / 60.0F, 2.0F) - 1.0F));
        float m = l - (c / 2.0F);

        float r = 0.0F;
        float g = 0.0F;
        float b = 0.0F;

        if (h < 60.0F) {
            r = c;
            g = x;
            b = 0.0F;
        } else if (h < 120.0F) {
            r = x;
            g = c;
            b = 0.0F;
        } else if (h < 180.0F) {
            r = 0.0F;
            g = c;
            b = x;
        } else if (h < 240.0F) {
            r = 0.0F;
            g = x;
            b = c;
        } else if (h < 300.0F) {
            r = x;
            g = 0.0F;
            b = c;
        } else {
            r = c;
            g = 0.0F;
            b = x;
        }
        return {r + m, g + m, b + m, 1.0F};
    }

    static glm::vec4 fromHSLA(float h, float s, float l, float a) {
        glm::vec4 rgb = fromHSL(h, s, l);
        rgb[4] = a;
        return rgb;
    }
};
} // namespace dzemikk

#endif // DZEMIKK_COLOR_H
