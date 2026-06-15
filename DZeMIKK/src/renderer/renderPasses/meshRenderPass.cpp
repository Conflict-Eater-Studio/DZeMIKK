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
                    if (b.mesh == mesh && b.material == material && b.color == glm::vec3(r->getColor().x, r->getColor().y, r->getColor().z) ){
                        batch = &b;
                        break;
                    }
                }

                if (!batch) {
                    _batches.push_back({});
                    batch = &_batches.back();

                    batch->mesh = mesh;
                    batch->material = material;
                    batch->color = glm::vec3(r->getColor().x, r->getColor().y, r->getColor().z);

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

            if (material->getAlbedoTexture()) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material->getAlbedoTexture()->getId());
                shader->setSampler("diffuseTexture", 0);
            }

            shader->setInt("useTexture", material->getAlbedoTexture() ? 1 : 0);

            shader->setFloat("shininess", 6.0f);
            shader->setFloat("specularStrength", .5f);
            shader->setVec3("objectColor", batch.color);
            shader->setVec3("viewPos", ctx.sceneCamera->getOwner()->transform()->getPosition());

            mesh->drawInstanced(batch.models, batch.instanceVBO);
            glBindTexture(GL_TEXTURE_2D, 0);
            Profiler::Get().stats.drawCalls++;
        }
    }
}
