#include "ecs/components/outlinePostProcessEffect.h"
#include <GLFW/glfw3.h>

void dzemikk::OutlinePostProcessEffect::bindShaderUniforms(Shader& shader) {
    PostProcessEffect::bindShaderUniforms(shader);

    GLFWwindow* window = glfwGetCurrentContext();
    int width = 0;
    int height = 0;

    if (window != nullptr) {
        glfwGetFramebufferSize(window, &width, &height);
    }

    if (width <= 0 || height <= 0) {
        width = 1;
        height = 1;
    }

    shader.setVec2("resolution", glm::vec2(static_cast<float>(width), static_cast<float>(height)));
    shader.setVec3("color", _color);
}
std::string dzemikk::OutlinePostProcessEffect::typeName() const {
    return "OutlinePostProcessEffect";
}
void dzemikk::OutlinePostProcessEffect::setColor(glm::vec3 color) {
    _color = color;
}