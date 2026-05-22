#include "renderer/renderPasses/imageRenderPass.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

#include "ecs/componentRegistry.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/rectTransform.h"
#include "ecs/gameobject.h"

#include "core/profiler.h"

void dzemikk::ImageRenderPass::execute(RenderContext& ctx) {
    if (ctx.uiCamera)
        ctx.uiProjection = ctx.uiCamera->getProjection();
    glDisable(GL_DEPTH_TEST);

    std::vector<ImageRenderer*> uiSprites;
    ComponentRegistry::get().getEnabledComponents<ImageRenderer>(uiSprites);
    try
    {
        DZ_PROFILE_GPU("Image Rendering");
        std::ranges::sort(uiSprites, [](ImageRenderer* a, ImageRenderer* b) {
            unsigned int az = a->getOwner()->rectTransform()->getZIndex();
            unsigned int bz = a->getOwner()->rectTransform()->getZIndex();
            return az < bz;
        });
        for (auto* r : uiSprites) {
            if (!r->isValid()) {
                continue;
            }

            Shader* shader = r->getMaterial()->getShader();

            if (!shader) {
                continue;
            }

            shader->bind();

            shader->setMat4("model", r->getRectTransform()->getWorldMatrix());
            shader->setMat4("projection", ctx.uiProjection);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, r->getTexture());
            //shader->setInt("spriteTexture", 0);
            shader->setBool("useTexture", false);
            shader->setVec4("spriteColor", r->getColor());

            r->getMesh()->draw();
            Profiler::Get().stats.drawCalls++;

            Profiler::Get().stats.renderedObjects++;
            Profiler::Get().stats.vertexCount += r->getMesh()->getVertexCount();
            Profiler::Get().stats.triangleCount += r->getMesh()->getVertexCount() / 3;
        }
    } catch (...) {
    
    }
}
