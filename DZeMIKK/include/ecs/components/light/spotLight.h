#ifndef DZEMIKK_SPOT_LIGHT_H
#define DZEMIKK_SPOT_LIGHT_H

#include "ecs/component.h"

#include <algorithm>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <string>

namespace dzemikk {

/**
 * @brief Cone-shaped spotlight.
 *
 * Emits light in a cone defined by inner/outer cutoff angles.
 */
class SpotLight final : public Component {
  public:
    SpotLight() = default;

    SpotLight(const glm::vec3& direction, float innerCutoff = 12.5f, float outerCutoff = 17.5f,
              const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f, float range = 15.0f,
              bool castsShadows = false)
        : _direction(glm::normalize(direction)), _innerCutoff(innerCutoff),
          _outerCutoff(outerCutoff), _color(color), _intensity(std::max(0.0f, intensity)),
          _range(std::max(0.0f, range)), _castsShadows(castsShadows) {}

    ~SpotLight() override = default;

  public:
    [[nodiscard]]
    const glm::vec3& getDirection() const noexcept {
        return _direction;
    }

    void setDirection(const glm::vec3& dir) {
        if (glm::length(dir) > 0.0f)
            _direction = glm::normalize(dir);
    }

    [[nodiscard]]
    float getInnerCutoff() const noexcept {
        return _innerCutoff;
    }

    void setInnerCutoff(float value) {
        _innerCutoff = std::clamp(value, 0.0f, 89.0f);
        if (_innerCutoff > _outerCutoff)
            _outerCutoff = _innerCutoff;
    }

    [[nodiscard]]
    float getOuterCutoff() const noexcept {
        return _outerCutoff;
    }

    void setOuterCutoff(float value) {
        _outerCutoff = std::clamp(value, 0.0f, 89.0f);
        if (_outerCutoff < _innerCutoff)
            _innerCutoff = _outerCutoff;
    }

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
        return "SpotLight";
    }

  private:
    glm::vec3 _direction{0.0f, -1.0f, 0.0f};

    float _innerCutoff{12.5f};
    float _outerCutoff{17.5f};

    glm::vec3 _color{1.0f, 1.0f, 1.0f};
    float _intensity{1.0f};

    float _range{15.0f};

    bool _castsShadows{false};
};

} // namespace dzemikk

#endif // DZEMIKK_SPOT_LIGHT_H