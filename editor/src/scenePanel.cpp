#include "scenePanel.h"

#include "renderer/framebuffer.h"
#include "renderer/renderer.h"

#include <imgui.h>

void editor::ScenePanel::draw(dzemikk::Renderer* renderer) {

    if (!renderer) {
        return;
    }

    ImGui::Begin("Scene");

    ImVec2 available = ImGui::GetContentRegionAvail();

    constexpr float aspectRatio = 16.0F / 9.0F;

    ImVec2 viewportSize;

    viewportSize.x = available.x;
    viewportSize.y = viewportSize.x / aspectRatio;

    if (viewportSize.y > available.y) {
        viewportSize.y = available.y;
        viewportSize.x = viewportSize.y * aspectRatio;
    }

    float offsetX = (available.x - viewportSize.x) * 0.5F;
    float offsetY = (available.y - viewportSize.y) * 0.5F;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

    if (_viewportSize.x != viewportSize.x || _viewportSize.y != viewportSize.y) {
        _viewportSize.x = viewportSize.x;
        _viewportSize.y = viewportSize.y;

        renderer->setViewportSize(static_cast<uint32_t>(_viewportSize.x),
                                  static_cast<uint32_t>(_viewportSize.y));
    }

    auto* framebuffer = renderer->getSceneFramebuffer();

    if (framebuffer) {
        uint32_t textureID = framebuffer->getColorAttachmentRendererID();
        ImGui::Image((ImTextureID)(uintptr_t)textureID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
    }

    ImGui::End();
}