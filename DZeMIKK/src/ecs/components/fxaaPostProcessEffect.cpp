#include "ecs/components/fxaaPostProcessEffect.h"

#include <GLFW/glfw3.h>

void dzemikk::FXAAPostProcessEffect::bindShaderUniforms(Shader& shader) {
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
    shader.setBool("showDifference", _showDifference);
    shader.setFloat("differenceGain", _differenceGain);
}

std::string dzemikk::FXAAPostProcessEffect::typeName() const {
    return "FXAAPostProcessEffect";
}
