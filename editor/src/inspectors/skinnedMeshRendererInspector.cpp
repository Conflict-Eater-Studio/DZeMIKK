#include "inspectors/skinnedMeshRendererInspector.h"
#include "inspectors/inspectorRegistry.h"
#include "ui/propertyDrawer.h"

#include <ecs/components/skinnedMeshRenderer.h>
#include <ecs/gameobject.h>
#include <renderer/material.h>

#include <imgui.h>

void editor::SkinnedMeshRendererInspector::draw(dzemikk::SkinnedMeshRenderer* renderer,
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

    if (PropertyDrawer::drawMaterials("Materials", renderer, ctx)) {

    }
}