#include "inspectors/meshRendererInspector.h"

#include "ecs/gameobject.h"
#include "renderer/material.h"
#include "renderer/model.h"
#include "renderer/shader.h"
#include "ui/propertyDrawer.h"

#include <imgui.h>

using namespace dzemikk;

void editor::MeshRendererInspector::draw(MeshRenderer* renderer, const InspectorContext& ctx) {
    if (!renderer) {
        return;
    }

    if (!ImGui::CollapsingHeader("MeshRenderer", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    auto modelHandle = renderer->getModelHandle();

    if (PropertyDrawer::drawModel(renderer->getOwner()->getName(), modelHandle, ctx)) {
        renderer->setModel(modelHandle);
    }

    glm::vec4 color = renderer->getColor();

    if (ImGui::ColorEdit4("Color", &color.x)) {
        renderer->setColor(color);
    }

    const auto& materials = renderer->getMaterials();

    ImGui::Text("Materials: %zu", materials.size());

    for (size_t i = 0; i < materials.size(); i++) {

        ImGui::PushID(static_cast<int>(i));

        auto material = materials[i];

        if (!material) {

            ImGui::Text("Material %zu: null", i);

            ImGui::SameLine();

            std::string createButton = "Create##" + std::to_string(i);

            if (ImGui::Button(createButton.c_str())) {

                auto newMaterial = std::make_shared<Material>();

                renderer->setMaterial(i, newMaterial);

                ImGui::PopID();
                break;
            }

            ImGui::PopID();
            continue;
        }

        std::string header = "Material " + std::to_string(i);

        if (ImGui::TreeNode(header.c_str())) {

            auto shaderHandle = material->getShaderHandle();

            if (PropertyDrawer::drawShader("Shader", shaderHandle, ctx)) {
                material->setShader(shaderHandle);
            }

            std::string removeButton = "Remove##" + std::to_string(i);

            if (ImGui::Button(removeButton.c_str())) {

                renderer->setMaterial(i, nullptr);

                ImGui::TreePop();
                ImGui::PopID();

                continue;
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    if (ImGui::Button("Add Material")) {

        auto material = std::make_shared<Material>();

        renderer->setMaterial(materials.size(), material);
    }
}