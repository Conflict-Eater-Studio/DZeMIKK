#ifndef DZEMIKK_COLOR_GRADING_EFFECT_H
#define DZEMIKK_COLOR_GRADING_EFFECT_H

#include "ecs/components/postProcessEffect.h"
#include <glm/glm.hpp>

namespace dzemikk {

class ColorGradingEffect : public PostProcessEffect {
  public:
    using Base = PostProcessEffect;

    ColorGradingEffect() = default;
    ~ColorGradingEffect() override = default;

    void bindShaderUniforms(Shader& shader) override;
    std::string typeName() const override;

    // Exposure
    [[nodiscard]] float getExposure() const { return _exposure; }
    void setExposure(float val) { _exposure = val; }

    // Contrast
    [[nodiscard]] float getContrast() const { return _contrast; }
    void setContrast(float val) { _contrast = val; }

    // Saturation
    [[nodiscard]] float getSaturation() const { return _saturation; }
    void setSaturation(float val) { _saturation = val; }

    // Temperature
    [[nodiscard]] float getTemperature() const { return _temperature; }
    void setTemperature(float val) { _temperature = val; }

    // Tint
    [[nodiscard]] float getTint() const { return _tint; }
    void setTint(float val) { _tint = val; }

    // Color Filter
    [[nodiscard]] const glm::vec3& getColorFilter() const { return _colorFilter; }
    void setColorFilter(const glm::vec3& filter) { _colorFilter = filter; }

  private:
    float _exposure = 0.0f;
    float _contrast = 1.0f;
    float _saturation = 1.0f;
    float _temperature = 0.0f;
    float _tint = 0.0f;
    glm::vec3 _colorFilter = glm::vec3(1.0f);
};

} // namespace dzemikk

#endif // DZEMIKK_COLOR_GRADING_EFFECT_H
