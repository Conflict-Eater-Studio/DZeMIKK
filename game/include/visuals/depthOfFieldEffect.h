#ifndef DZEMIKK_DEPTH_OF_FIELD_EFFECT_H
#define DZEMIKK_DEPTH_OF_FIELD_EFFECT_H

#include "assetManager/assetHandle.h"
#include "renderer/shader.h"
#include "ecs/components/postProcessEffect.h"

namespace dzemikk {

class DepthOfFieldEffect : public PostProcessEffect {
  public:
    using Base = PostProcessEffect;

    DepthOfFieldEffect() = default;
    ~DepthOfFieldEffect() override = default;

    void bindShaderUniforms(Shader& shader) override;

    void setFocusDistance(float v) {
        _focusDistance = v;
    }
    void setFocusRange(float v) {
        _focusRange = v;
    }
    void setMaxBlur(float v) {
        _maxBlur = v;
    }

    float getFocusDistance() const {
        return _focusDistance;
    }
    float getFocusRange() const {
        return _focusRange;
    }
    float getMaxBlur() const {
        return _maxBlur;
    }

  private:
    float _focusDistance = 0.0f;
    float _focusRange = 1.0f;
    float _maxBlur = 10.0f;
};

} // namespace dzemikk

#endif