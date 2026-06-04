#ifndef DZEMIKK_DIRECTIONAL_LIGHT_H
#define DZEMIKK_DIRECTIONAL_LIGHT_H

#include "ecs/component.h"

#include <algorithm>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <string>

namespace dzemikk {

/**
 * @brief Infinite directional light component.
 *
 * Simulates light emitted from an infinitely distant source
 * like the sun. Only the direction matters.
 */
class DirectionalLight final : public Component {
  public:
    DirectionalLight() = default;

    DirectionalLight(const glm::vec3& direction, const glm::vec3& color = glm::vec3(1.0f),
                     float intensity = 1.0f, bool castsShadows = false)
        : _direction(glm::normalize(direction)), _color(color),
          _intensity(std::max(0.0f, intensity)), _castsShadows(castsShadows) {}

    ~DirectionalLight() override = default;

  public:
    /**
     * @brief Returns normalized light direction.
     */
    [[nodiscard]]
    const glm::vec3& getDirection() const noexcept {
        return _direction;
    }

    /**
     * @brief Sets light direction.
     *
     * Direction is automatically normalized.
     */
    void setDirection(const glm::vec3& direction) {
        if (glm::length(direction) > 0.0f) {
            _direction = glm::normalize(direction);
        }
    }

    /**
     * @brief Returns light color.
     */
    [[nodiscard]]
    const glm::vec3& getColor() const noexcept {
        return _color;
    }

    /**
     * @brief Sets light color.
     */
    void setColor(const glm::vec3& color) {
        _color = color;
    }

    /**
     * @brief Returns light intensity.
     */
    [[nodiscard]]
    float getIntensity() const noexcept {
        return _intensity;
    }

    /**
     * @brief Sets light intensity.
     *
     * Negative values are clamped to 0.
     */
    void setIntensity(float intensity) {
        _intensity = std::max(0.0f, intensity);
    }

    /**
     * @brief Returns whether the light casts shadows.
     */
    [[nodiscard]]
    bool castsShadows() const noexcept {
        return _castsShadows;
    }

    /**
     * @brief Enables or disables shadow casting.
     */
    void setCastsShadows(bool enabled) noexcept {
        _castsShadows = enabled;
    }

  public:
    /**
     * @brief Runtime component type name.
     */
    [[nodiscard]]
    std::string typeName() const override {
        return "DirectionalLight";
    }

  private:
    /**
     * @brief Normalized light direction.
     */
    glm::vec3 _direction{0.0f, -1.0f, 0.0f};

    /**
     * @brief RGB light color in linear space.
     */
    glm::vec3 _color{1.0f, 1.0f, 1.0f};

    /**
     * @brief Light intensity multiplier.
     */
    float _intensity{1.0f};

    /**
     * @brief Shadow casting flag.
     */
    bool _castsShadows{false};
};

} // namespace dzemikk

#endif // DZEMIKK_DIRECTIONAL_LIGHT_H