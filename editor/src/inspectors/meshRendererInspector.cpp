#include "inspectors/meshRendererInspector.h"
#include "inspectors/inspectorRegistry.h"
#include "ui/propertyDrawer.h"

#include <ecs/gameobject.h>
#include <ecs/components/meshRenderer.h>
#include <renderer/material.h>
#include <renderer/model.h>
#include <renderer/shader.h>

#include <imgui.h>

void editor::MeshRendererInspector::draw(dzemikk::MeshRenderer* renderer, const InspectorContext& ctx) {
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

    if (PropertyDrawer::drawColor(renderer->getOwner()->getName(), color)) {
        renderer->setColor(color);
    }

    PropertyDrawer::drawMaterials(renderer->getOwner()->getName(), renderer, ctx);
}