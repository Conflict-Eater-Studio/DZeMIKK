#include "ecs/components/antiAliasingEffect.h"

namespace dzemikk {

std::string AntiAliasingEffect::typeName() const {
    return "AntiAliasingEffect";
}

void AntiAliasingEffect::bindShaderUniforms(Shader& shader) {
    shader.setFloat("uSpanMax", _spanMax);
    shader.setFloat("uReduceMul", _reduceMul);
    shader.setFloat("uReduceMin", _reduceMin);
}

} // namespace dzemikk
