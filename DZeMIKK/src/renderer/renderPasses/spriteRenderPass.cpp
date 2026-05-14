#include "renderer/renderPasses/spriteRenderPass.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "renderer/texture.h"
#include "renderer/mesh.h"

#include "ecs/componentRegistry.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/transform.h"

#include "core/profiler.h"

void dzemikk::SpriteRenderPass::execute(RenderContext& ctx) {
    dzemikk::ComponentRegistry::get().getEnabledComponents<SpriteRenderer>(_spriteRenderers);

    {
        DZ_PROFILE_GPU("Transparent Rendering (Sprites)");
        for (auto* r : _spriteRenderers) {
            if (!r->isValid())
                continue;

            Shader* shader = r->getMaterial()->getShader();
            shader->bind();

            shader->setMat4("model", r->getTransform()->getWorldMatrix());
            shader->setMat4("projection", ctx.uiProjection);

            if (r->hasTexture()) {
                r->getTexture()->bind(0);
                shader->setInt("spriteTexture", 0);
                shader->setBool("useTexture", true);
            } else {
                shader->setBool("useTexture", false);
            }

            shader->setVec4("spriteColor", r->getColor());

            r->getMesh()->draw();

            Profiler::Get().stats.drawCalls++;

            Profiler::Get().stats.renderedObjects++;
            Profiler::Get().stats.vertexCount += r->getMesh()->getVertexCount();
            Profiler::Get().stats.triangleCount += r->getMesh()->getVertexCount() / 3;
        }
    }
}
