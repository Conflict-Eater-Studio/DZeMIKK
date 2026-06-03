#include "ecs/components/outlinePostProcessEffect.h"
#include <GLFW/glfw3.h>

void dzemikk::OutlinePostProcessEffect::bindShaderUniforms(Shader& shader) {
    PostProcessEffect::bindShaderUniforms(shader);
    shader.setVec3("color", _color);
}
std::string dzemikk::OutlinePostProcessEffect::typeName() const {
    return "OutlinePostProcessEffect";
}
void dzemikk::OutlinePostProcessEffect::setColor(glm::vec3 color) {
    _color = color;
}