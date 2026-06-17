#include "renderer/renderPasses/skinnedRenderPass.h"
#include "renderer/shader.h"

#include "ecs/componentRegistry.h"
#include "ecs/components/skinnedMeshRenderer.h"
#include "ecs/gameobject.h"

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

                Material* mat = r->getMaterial(i).get();
                if (!mat)
                    mat = r->getMaterial(0).get();

                if (!mat)
                    continue;

                Shader* shader = mat->getShader();

                if (!shader) {
                    continue;
                }

                shader->bind();

                shader->setMat4("model", transform->getWorldMatrix());
                shader->setMat4Array("u_Bones", bones);

                shader->setVec3("viewPos", ctx.sceneCamera->getOwner()->transform()->getPosition());

                shader->setVec3("albedoColor", mat->getAlbedoColor());
                shader->setFloat("metallic", mat->getMetallic());
                shader->setFloat("roughness", mat->getRoughness());
                shader->setFloat("ao", mat->getAO());
                shader->setVec3("emissiveColor", mat->getEmissiveColor());
                shader->setFloat("emissiveStrength", mat->getEmissiveStrength());

                if (mat->getAlbedoTexture()) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, mat->getAlbedoTexture()->getId());

                    shader->setSampler("albedoMap", 0);
                    shader->setInt("hasAlbedoMap", 1);
                } else {
                    shader->setInt("hasAlbedoMap", 0);
                }

                if (mat->getMetallicTexture()) {
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, mat->getMetallicTexture()->getId());

                    shader->setSampler("metallicMap", 1);
                    shader->setInt("hasMetallicMap", 1);
                } else {
                    shader->setInt("hasMetallicMap", 0);
                }

                if (mat->getRoughnessTexture()) {
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, mat->getRoughnessTexture()->getId());

                    shader->setSampler("roughnessMap", 2);
                    shader->setInt("hasRoughnessMap", 1);
                } else {
                    shader->setInt("hasRoughnessMap", 0);
                }

                if (mat->getAOTexture()) {
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, mat->getAOTexture()->getId());

                    shader->setSampler("aoMap", 3);
                    shader->setInt("hasAOMap", 1);
                } else {
                    shader->setInt("hasAOMap", 0);
                }

                if (mat->getNormalTexture()) {
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, mat->getNormalTexture()->getId());

                    shader->setSampler("normalMap", 4);
                    shader->setInt("hasNormalMap", 1);
                } else {
                    shader->setInt("hasNormalMap", 0);
                }

                if (mat->getEmissiveTexture()) {
                    glActiveTexture(GL_TEXTURE5);
                    glBindTexture(GL_TEXTURE_2D, mat->getEmissiveTexture()->getId());

                    shader->setSampler("emissiveMap", 5);
                    shader->setInt("hasEmissiveMap", 1);
                } else {
                    shader->setInt("hasEmissiveMap", 0);
                }

                sub->mesh->draw();

                for (int i = 0; i < 6; i++) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }

                Profiler::Get().stats.drawCalls++;
                Profiler::Get().stats.renderedObjects++;
                Profiler::Get().stats.vertexCount += sub->mesh->getVertexCount();
                Profiler::Get().stats.triangleCount += sub->mesh->getVertexCount() / 3;
            }
        }
    }
}
