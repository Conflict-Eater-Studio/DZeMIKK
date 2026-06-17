#include "renderer/renderPasses/imageRenderPass.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

#include "ecs/componentRegistry.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/rectTransform.h"
#include "ecs/gameobject.h"
#include "renderer/texture.h"

#include "core/profiler.h"

void dzemikk::ImageRenderPass::execute(RenderContext& ctx) {
    if (ctx.uiCamera)
        ctx.uiProjection = ctx.uiCamera->getProjection();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::vector<ImageRenderer*> uiSprites;
    ComponentRegistry::get().getEnabledComponents<ImageRenderer>(uiSprites);

    {
        DZ_PROFILE_GPU("Image Rendering");
        std::ranges::sort(uiSprites, [](ImageRenderer* a, ImageRenderer* b) {
            unsigned int az = a->getOwner()->rectTransform()->getZIndex();
            unsigned int bz = b->getOwner()->rectTransform()->getZIndex();
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

            if (r->hasTexture()) {
                r->getTexture()->bind(0);
                shader->setInt("spriteTexture", 0);
                shader->setBool("useTexture", true);
            } else {
                shader->setBool("useTexture", false);
            }

            shader->setMat4("model", r->getRectTransform()->getWorldMatrix());
            shader->setMat4("projection", ctx.uiProjection);
            shader->setVec4("spriteColor", r->getColor());

            r->getMesh()->draw();
            Profiler::Get().stats.drawCalls++;

            Profiler::Get().stats.renderedObjects++;
            Profiler::Get().stats.vertexCount += r->getMesh()->getVertexCount();
            Profiler::Get().stats.triangleCount += r->getMesh()->getVertexCount() / 3;
        }
    } 
}
