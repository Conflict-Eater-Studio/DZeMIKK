#include "ecs/components/colorGradingEffect.h"

namespace dzemikk {

std::string ColorGradingEffect::typeName() const {
    return "ColorGradingEffect";
}

void ColorGradingEffect::bindShaderUniforms(Shader& shader) {
    shader.setFloat("uExposure", _exposure);
    shader.setFloat("uContrast", _contrast);
    shader.setFloat("uSaturation", _saturation);
    shader.setFloat("uTemperature", _temperature);
    shader.setFloat("uTint", _tint);
    shader.setVec3("uColorFilter", _colorFilter);
}

} // namespace dzemikk
