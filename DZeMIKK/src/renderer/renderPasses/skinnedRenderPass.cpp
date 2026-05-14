#include "renderer/renderPasses/skinnedRenderPass.h"
#include "renderer/shader.h"

#include "ecs/componentRegistry.h"
#include "ecs/components/skinnedMeshRenderer.h"

#include "core/profiler.h"


void dzemikk::SkinnedRenderPass::execute(RenderContext& ctx) {
    ComponentRegistry::get().getEnabledComponents<SkinnedMeshRenderer>(_skinnedRenderers);

    {
        DZ_PROFILE_GPU("Skinned Rendering");

        for (auto* r : _skinnedRenderers) {
            if (!r)
                continue;

            Model* model = r->getModel().get();
            Transform* transform = r->getTransform();

            if (!model || !transform)
                continue;

            auto skeleton = model->getSkeleton();
            if (!skeleton)
                continue;

            auto& bones = r->getBoneMatrices();

            if (bones.size() != skeleton->getBoneCount()) {
                bones.resize(skeleton->getBoneCount(), glm::mat4(1.0f));
            }

            r->calculateBoneMatrices(0, glm::mat4(1.0f));

            for (size_t i = 0; i < model->getSubMeshes().size(); i++) {

                const auto* sub = model->getSubMesh(i);
                if (!sub)
                    continue;

                Material* mat = r->getMaterial(i);
                if (!mat)
                    continue;

                Shader* shader = mat->getShader();
                shader->bind();

                shader->setMat4("model", transform->getWorldMatrix());
                shader->setVec3("lightDir", glm::vec3(1.0f, -1.0f, -1.0f));
                shader->setVec3("lightColor", glm::vec3(1.0f));
                shader->setMat4Array("u_Bones", bones);

                sub->mesh->draw();

                Profiler::Get().stats.drawCalls++;
                Profiler::Get().stats.renderedObjects++;
                Profiler::Get().stats.vertexCount += sub->mesh->getVertexCount();
                Profiler::Get().stats.triangleCount += sub->mesh->getVertexCount() / 3;
            }
        }
    }
}
