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

            if (material->getTexture()) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material->getTexture()->getId());
                shader->setInt("texture", 0);
            }

            shader->setInt("hasTexture", material->getTexture() ? 1 : 0);

            shader->setFloat("shininess", 6.0f);
            shader->setFloat("specularStrength", .5f);
            shader->setInt("dirLightCount", ctx.directionalCount);
            shader->setInt("pointLightCount", ctx.pointCount);
            shader->setInt("spotLightCount", ctx.spotCount);
            shader->setVec3("objectColor", batch.color);
            shader->setVec3("viewPos", ctx.sceneCamera->getOwner()->transform()->getPosition());

            for (int i = 0; i < ctx.directionalCount; i++) {
                std::string dirName = "dirDirection[" + std::to_string(i) + "]";
                std::string colName = "dirColor[" + std::to_string(i) + "]";
                std::string intName = "dirIntensity[" + std::to_string(i) + "]";

                shader->setVec3(dirName.c_str(), ctx.directionalLights[i].direction);

                shader->setVec3(colName.c_str(), ctx.directionalLights[i].color);

                shader->setFloat(intName.c_str(), ctx.directionalLights[i].color.a);
            }

            for (int i = 0; i < ctx.pointCount; i++) {
                std::string posName = "pointPos[" + std::to_string(i) + "]";
                std::string colName = "pointColor[" + std::to_string(i) + "]";
                std::string intName = "pointIntensity[" + std::to_string(i) + "]";
                std::string rangeName = "pointRange[" + std::to_string(i) + "]";

                shader->setVec3(posName.c_str(), glm::vec3(ctx.pointLights[i].position));

                shader->setVec3(colName.c_str(), glm::vec3(ctx.pointLights[i].color));

                shader->setFloat(intName.c_str(), ctx.pointLights[i].color.a);

                shader->setFloat(rangeName.c_str(), ctx.pointLights[i].params.x);
            }

            for (int i = 0; i < ctx.spotCount; i++) {
                std::string posName = "spotPos[" + std::to_string(i) + "]";
                std::string dirName = "spotDir[" + std::to_string(i) + "]";
                std::string colName = "spotColor[" + std::to_string(i) + "]";
                std::string intName = "spotIntensity[" + std::to_string(i) + "]";
                std::string innerName = "spotInner[" + std::to_string(i) + "]";
                std::string outerName = "spotOuter[" + std::to_string(i) + "]";

                shader->setVec3(posName.c_str(), glm::vec3(ctx.spotLights[i].position));

                shader->setVec3(dirName.c_str(), glm::vec3(ctx.spotLights[i].direction));

                shader->setVec3(colName.c_str(), glm::vec3(ctx.spotLights[i].color));

                shader->setFloat(intName.c_str(), ctx.spotLights[i].color.a);

                shader->setFloat(innerName.c_str(), ctx.spotLights[i].params.y);

                shader->setFloat(outerName.c_str(), ctx.spotLights[i].params.z);
            }

            mesh->drawInstanced(batch.models, batch.instanceVBO);
            glBindTexture(GL_TEXTURE_2D, 0);
            Profiler::Get().stats.drawCalls++;
        }
    }
}
