#pragma once
#ifndef DZEMIKK_FXAA_POST_PROCESS_EFFECT_H
#define DZEMIKK_FXAA_POST_PROCESS_EFFECT_H

#include "ecs/components/postProcessEffect.h"

namespace dzemikk {

class FXAAPostProcessEffect : public PostProcessEffect {
  public:
    using Base = PostProcessEffect;

    FXAAPostProcessEffect() = default;
    ~FXAAPostProcessEffect() override = default;

    void bindShaderUniforms(Shader& shader) override;

    std::string typeName() const override;

    void setShowDifference(bool value) {
        _showDifference = value;
    }

    [[nodiscard]] bool getShowDifference() const {
        return _showDifference;
    }

    void setDifferenceGain(float value) {
        _differenceGain = value;
    }

    [[nodiscard]] float getDifferenceGain() const {
        return _differenceGain;
    }

  private:
    bool _showDifference = false;
    float _differenceGain = 8.0f;
};

} // namespace dzemikk

#endif // DZEMIKK_FXAA_POST_PROCESS_EFFECT_H
