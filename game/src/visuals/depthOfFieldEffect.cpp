#include "visuals/depthOfFieldEffect.h"
#include "renderer/shader.h"

#include <iostream>

void dzemikk::DepthOfFieldEffect::bindShaderUniforms(Shader& shader) {
    shader.setFloat("uFocusDistance", _focusDistance);
    shader.setFloat("uFocusRange", _focusRange);
    shader.setFloat("uMaxBlur", _maxBlur);

    shader.setFloat("uNearPlane", 0.1f);
    shader.setFloat("uFarPlane", 1000.0f);
}