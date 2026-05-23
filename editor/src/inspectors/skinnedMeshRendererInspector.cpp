#include "inspectors/skinnedMeshRendererInspector.h"
#include "inspectors/inspectorRegistry.h"
#include "ecs/components/skinnedMeshRenderer.h"
#include "ecs/gameobject.h"
#include "renderer/material.h"
#include "ui/propertyDrawer.h"


#include <imgui.h>

using namespace dzemikk;

void editor::SkinnedMeshRendererInspector::draw(SkinnedMeshRenderer* renderer,
                                                const InspectorContext& ctx) {

    if (!renderer) {
        return;
    }

    if (!ImGui::CollapsingHeader("SkinnedMeshRenderer", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    auto modelHandle = renderer->getModel();

    if (PropertyDrawer::drawModel(renderer->getOwner()->getName(), modelHandle, ctx)) {

        renderer->setModel(modelHandle);
    }

    auto& materials = renderer->getMaterials();

    ImGui::Text("Materials: %zu", materials.size());

    for (size_t i = 0; i < materials.size(); i++) {

        ImGui::PushID(static_cast<int>(i));

        Material* material = materials[i];

        std::string header = "Material " + std::to_string(i);

        if (!material) {

            ImGui::Text("Material %zu: null", i);

            ImGui::SameLine();

            std::string createButton = "Create##" + std::to_string(i);

            if (ImGui::Button(createButton.c_str())) {

                material = new Material();

                renderer->setMaterial(i, material);
            }

            ImGui::PopID();
            continue;
        }

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

                return;
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    if (ImGui::Button("Add Material")) {

        renderer->setMaterial(materials.size(), new Material());
    }
}