#include "inspectors/spriteRendererInspector.h"

#include "ecs/components/spriteRenderer.h"
#include "renderer/material.h"
#include "ui/propertyDrawer.h"
#include "inspectors/inspectorRegistry.h"
#include <imgui.h>

void editor::SpriteRendererInspector::draw(dzemikk::SpriteRenderer* spriteRenderer,
                                           const InspectorContext& ctx) {

    if (!spriteRenderer) {
        return;
    }

    if (ImGui::CollapsingHeader("SpriteRenderer", ImGuiTreeNodeFlags_DefaultOpen)) {

        glm::vec4 color = spriteRenderer->getColor();

        if (PropertyDrawer::drawColor("Color", color)) {
            spriteRenderer->setColor(color);
        }

        auto* material = spriteRenderer->getMaterial();

        if (!material) {

            ImGui::TextDisabled("Material: null");

            if (ImGui::Button("Create Material")) {

                auto* newMaterial = new dzemikk::Material();

                spriteRenderer->setMaterial(newMaterial);

                material = newMaterial;
            }
        }

        if (material) {

            auto shaderHandle = material->getShaderHandle();

            if (PropertyDrawer::drawShader("Shader", shaderHandle, ctx)) {

                material->setShader(shaderHandle);
            }
        }

        bool useTexture = spriteRenderer->useTexture();

        if (PropertyDrawer::drawBool("Use Texture", useTexture)) {
            spriteRenderer->setUseTexture(useTexture);
        }

        if (useTexture) {

            auto textureHandle = spriteRenderer->getTextureHandle();

            if (PropertyDrawer::drawTexture("Texture", textureHandle, ctx)) {
                spriteRenderer->setTexture(textureHandle);
            }
        }
    }
}