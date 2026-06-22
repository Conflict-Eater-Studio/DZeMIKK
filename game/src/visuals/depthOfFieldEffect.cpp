#include "visuals/depthOfFieldEffect.h"
#include "renderer/shader.h"

#include "renderer/shader.h"
#include <iostream>

#include <imgui.h>

void dzemikk::DepthOfFieldEffect::bindShaderUniforms(Shader& shader) {
    shader.setFloat("uFocusDistance", _focusDistance);
    shader.setFloat("uFocusRange", _focusRange);
    shader.setFloat("uMaxBlur", _maxBlur);

    shader.setFloat("uNearPlane", 0.1f);
    shader.setFloat("uFarPlane", 1000.0f);

    ImGui::Begin("DOF");

    ImGui::InputFloat("Focus Distance", &_focusDistance);
    ImGui::InputFloat("Focus Range", &_focusRange);
    ImGui::InputFloat("Max Blur", &_maxBlur);

    ImGui::Text("WARNING: debug inside bindShaderUniforms");

    ImGui::End();
}