#ifndef DZEMIKK_POINT_LIGHT_H
#define DZEMIKK_POINT_LIGHT_H

#include "ecs/component.h"

#include <algorithm>
#include <glm/vec3.hpp>
#include <string>

namespace dzemikk {

/**
 * @brief Omnidirectional point light.
 *
 * Emits light equally in all directions from a single point in space.
 */
class PointLight final : public Component {
  public:
    PointLight() = default;

    PointLight(const glm::vec3& color, float intensity = 1.0f, float range = 10.0f,
               bool castsShadows = false)
        : _color(color), _intensity(std::max(0.0f, intensity)), _range(std::max(0.0f, range)),
          _castsShadows(castsShadows) {}

    ~PointLight() override = default;

  public:
    [[nodiscard]]
    const glm::vec3& getColor() const noexcept {
        return _color;
    }

    void setColor(const glm::vec3& color) {
        _color = color;
    }

    [[nodiscard]]
    float getIntensity() const noexcept {
        return _intensity;
    }

    void setIntensity(float intensity) {
        _intensity = std::max(0.0f, intensity);
    }

    [[nodiscard]]
    float getRange() const noexcept {
        return _range;
    }

    void setRange(float range) {
        _range = std::max(0.0f, range);
    }

    [[nodiscard]]
    bool castsShadows() const noexcept {
        return _castsShadows;
    }

    void setCastsShadows(bool enabled) noexcept {
        _castsShadows = enabled;
    }

  public:
    [[nodiscard]]
    std::string typeName() const override {
        return "PointLight";
    }

  private:
    glm::vec3 _color{1.0f, 1.0f, 1.0f};
    float _intensity{1.0f};
    float _range{10.0f};
    bool _castsShadows{false};
};

} // namespace dzemikk

#endif // DZEMIKK_POINT_LIGHT_H