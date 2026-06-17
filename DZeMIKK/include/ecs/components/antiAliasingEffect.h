#ifndef DZEMIKK_ANTI_ALIASING_EFFECT_H
#define DZEMIKK_ANTI_ALIASING_EFFECT_H

#include "ecs/components/postProcessEffect.h"

namespace dzemikk {

class AntiAliasingEffect : public PostProcessEffect {
  public:
    using Base = PostProcessEffect;

    AntiAliasingEffect() = default;
    ~AntiAliasingEffect() override = default;

    void bindShaderUniforms(Shader& shader) override;
    std::string typeName() const override;

    // Span Max (limits search distance)
    [[nodiscard]] float getSpanMax() const { return _spanMax; }
    void setSpanMax(float val) { _spanMax = val; }

    // Reduce Mul
    [[nodiscard]] float getReduceMul() const { return _reduceMul; }
    void setReduceMul(float val) { _reduceMul = val; }

    // Reduce Min
    [[nodiscard]] float getReduceMin() const { return _reduceMin; }
    void setReduceMin(float val) { _reduceMin = val; }

  private:
    float _spanMax = 8.0f;
    float _reduceMul = 0.125f;    // 1.0f / 8.0f
    float _reduceMin = 0.0078125f; // 1.0f / 128.0f
};

} // namespace dzemikk

#endif // DZEMIKK_ANTI_ALIASING_EFFECT_H
