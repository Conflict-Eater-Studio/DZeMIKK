#include "inspectors/imageRendererInspector.h"
#include "ui/propertyDrawer.h"

#include <renderer/material.h>
#include <ecs/components/ui/imageRenderer.h>

#include <imgui.h>

void editor::ImageRendererInspector::draw(dzemikk::ImageRenderer* renderer,
                                          const InspectorContext& ctx) {
    if (!renderer) {
        return;
    }

    if (ImGui::CollapsingHeader("ImageRenderer", ImGuiTreeNodeFlags_DefaultOpen)) {
        glm::vec4 color = renderer->getColor();

        if (PropertyDrawer::drawColor("Color", color)) {
            renderer->setColor(color);
        }

        auto material = renderer->getMaterial();

        if (!material) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Material is null!");

            if (ImGui::Button("Create Material")) {
                renderer->setMaterial(std::make_shared<dzemikk::Material>());
            }

            return;
        }

        auto shaderHandle = material->getShaderHandle();

        if (PropertyDrawer::drawShader("Shader", shaderHandle, ctx)) {
            material->setShader(shaderHandle);
        }

        bool useTexture = renderer->useTexture();

        if (PropertyDrawer::drawBool("Use Texture", useTexture)) {
            renderer->setUseTexture(useTexture);
        }

        if (useTexture) {
            auto textureHandle = renderer->getTextureHandle();

            if (PropertyDrawer::drawTexture("Texture", textureHandle, ctx)) {
                renderer->setTexture(textureHandle);
            }
        }
    }
}