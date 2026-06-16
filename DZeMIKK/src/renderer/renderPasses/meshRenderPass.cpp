#include "renderer/renderPasses/meshRenderPass.h"
#include "renderer/model.h"
#include "renderer/shader.h"

#include "ecs/gameobject.h"
#include "ecs/componentRegistry.h"
#include "ecs/components/transform.h"

#include "core/profiler.h"


void dzemikk::MeshRenderPass::execute(RenderContext& ctx) {
    buildMeshBatches(ctx);
    renderMeshBatches(ctx);
}

dzemikk::MeshRenderPass::~MeshRenderPass() {
    for (auto& batch : _batches) {
        if (batch.instanceVBO != 0) {
            glDeleteBuffers(1, &batch.instanceVBO);
            batch.instanceVBO = 0;
        }
    }

    _batches.clear();
}

void dzemikk::MeshRenderPass::buildMeshBatches(RenderContext& ctx) {
    dzemikk::ComponentRegistry::get().getEnabledComponents<MeshRenderer>(_meshRenderers);

    for (auto& batch : _batches) {
        batch.models.clear();
    }

    {
        DZ_PROFILE_CPU("Frustum Culling & Batching");
        for (auto* r : _meshRenderers) {
            if (!r->isValid())
                continue;

            Model* model = r->getModel().get();
            glm::mat4 transform = r->getTransform()->getWorldMatrix();

            float radius = r->getCullingRadius();

            if (!ctx.frustum->isSphereVisible(r->getTransform()->getPosition(), radius))
                continue;

            for (size_t i = 0; i < model->getSubMeshes().size(); i++) {
                const auto* sub = model->getSubMesh(i);
                if (!sub)
                    continue;

                Mesh* mesh = sub->mesh.get();

                Material* material = nullptr;

                if (i < r->getMaterials().size())
                    material = r->getMaterial(i);

                if (!material && !r->getMaterials().empty())
                    material = r->getMaterial(0);

                if (!material)
                    continue;

                Batch* batch = nullptr;

                for (auto& b : _batches) {
                    if (b.mesh == mesh && *b.material == *material) {
                        batch = &b;
                        break;
                    }
                }

                if (!batch) {
                    _batches.push_back({});
                    batch = &_batches.back();

                    batch->mesh = mesh;
                    batch->material = material;

                    glGenBuffers(1, &batch->instanceVBO);
                }

                batch->models.push_back(transform);
                Profiler::Get().stats.renderedObjects++;
                Profiler::Get().stats.vertexCount += batch->mesh->getVertexCount();
                Profiler::Get().stats.triangleCount += batch->mesh->getVertexCount() / 3;
            }
        }
    }
}

void dzemikk::MeshRenderPass::renderMeshBatches(RenderContext& ctx) {
    {
        DZ_PROFILE_GPU("Opaque Rendering (Batches)");

        if (!ctx.sceneCamera) {
            return;
        }

        for (auto& batch : _batches) {
            if (batch.models.empty())
                continue;

            Mesh* mesh = batch.mesh;
            Material* material = batch.material;
            Shader* shader = material->getShader();

            if (!shader) {
                continue;
            }

            shader->bind();

            shader->setVec3("viewPos", ctx.sceneCamera->getOwner()->transform()->getPosition());

            shader->setVec3("albedoColor", material->getAlbedoColor());
            shader->setFloat("metallic", material->getMetallic());
            shader->setFloat("roughness", material->getRoughness());
            shader->setFloat("ao", material->getAO());

            if (material->getAlbedoTexture()) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material->getAlbedoTexture()->getId());

                shader->setSampler("albedoMap", 0);
                shader->setInt("hasAlbedoMap", 1);
            } else {
                shader->setInt("hasAlbedoMap", 0);
            }

            if (material->getMetallicTexture()) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, material->getMetallicTexture()->getId());

                shader->setSampler("metallicMap", 1);
                shader->setInt("hasMetallicMap", 1);
            } else {
                shader->setInt("hasMetallicMap", 0);
            }

            if (material->getRoughnessTexture()) {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, material->getRoughnessTexture()->getId());

                shader->setSampler("roughnessMap", 2);
                shader->setInt("hasRoughnessMap", 1);
            } else {
                shader->setInt("hasRoughnessMap", 0);
            }

            if (material->getAOTexture()) {
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, material->getAOTexture()->getId());

                shader->setSampler("aoMap", 3);
                shader->setInt("hasAOMap", 1);
            } else {
                shader->setInt("hasAOMap", 0);
            }

            if (material->getNormalTexture()) {
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, material->getNormalTexture()->getId());

                shader->setSampler("normalMap", 4);
                shader->setInt("hasNormalMap", 1);
            } else {
                shader->setInt("hasNormalMap", 0);
            }

            mesh->drawInstanced(batch.models, batch.instanceVBO);

            for (int i = 0; i < 4; i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            Profiler::Get().stats.drawCalls++;
        }
    }
}
