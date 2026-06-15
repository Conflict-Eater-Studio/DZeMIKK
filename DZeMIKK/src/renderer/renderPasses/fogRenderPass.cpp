#include "renderer/renderPasses/fogRenderPass.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "renderer/texture.h"
#include "renderer/mesh.h"
#include "renderer/model.h"

#include "ecs/gameobject.h"
#include "ecs/componentRegistry.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/transform.h"

#include "core/profiler.h"
#include <GLFW/glfw3.h>

float g_FogDensity = 15.0f;
float g_FogNoiseScale = 0.35f;
float g_FogWindSpeed = 1.0f;
float g_FogNoiseContrast = 0.5f;
float g_FogWarpStrength = 1.5f;

void dzemikk::FogRenderPass::execute(RenderContext& ctx) {
    if (!ctx.sceneCamera) {
        return;
    }

    // Retrieve all MeshRenderers (enabled and disabled)
    dzemikk::ComponentRegistry::get().getComponents<MeshRenderer>(_meshRenderers);

    // Filter to those that use the fog volume shader
    std::vector<MeshRenderer*> fogRenderers;
    for (auto* r : _meshRenderers) {
        if (!r->isEnabled() || !r->isValid())
            continue;

        Model* model = r->getModel().get();
        if (!model)
            continue;

        for (size_t i = 0; i < model->getSubMeshes().size(); i++) {
            Material* material = nullptr;
            if (i < r->getMaterials().size())
                material = r->getMaterial(i);
            if (!material && !r->getMaterials().empty())
                material = r->getMaterial(0);

            if (material && material->getShaderHandle() &&
                material->getShaderHandle().getAssetPath() == "shaders/fog_volume") {
                fogRenderers.push_back(r);
                break;
            }
        }
    }

    if (fogRenderers.empty()) {
        return;
    }

    DZ_PROFILE_GPU("Fog Volume Rendering");

    // Save current OpenGL states
    GLboolean depthWriteEnabled;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteEnabled);

    GLint cullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &cullFaceMode);

    GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);

    // Configure states for transparency and back-face rendering
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT); // Render only back faces

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glm::vec2 screenResolution(static_cast<float>(viewport[2]), static_cast<float>(viewport[3]));

    float time = static_cast<float>(glfwGetTime());

    for (auto* r : fogRenderers) {
        Model* model = r->getModel().get();
        glm::mat4 modelMatrix = r->getTransform()->getWorldMatrix();

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

            Shader* shader = material->getShader();
            if (!shader)
                continue;

            shader->bind();

            // Bind depth texture
            if (ctx.depthTexture != 0) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, ctx.depthTexture);
                shader->setSampler("depthTexture", 1);
            }

            // Bind noise texture
            if (material->getTexture()) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material->getTexture()->getId());
                shader->setSampler("noiseTexture", 0);
                shader->setBool("useNoiseTexture", true);
            } else {
                shader->setBool("useNoiseTexture", false);
            }

            // Set matrices
            shader->setMat4("model", modelMatrix);
            shader->setMat4("invModel", glm::inverse(modelMatrix));
            shader->setMat4("invProjection", glm::inverse(ctx.sceneCamera->getProjection()));
            shader->setMat4("invView", glm::inverse(ctx.sceneCamera->getView()));

            // Set parameters
            shader->setVec3("viewPos", ctx.sceneCamera->getOwner()->transform()->getPosition());
            shader->setVec2("screenResolution", screenResolution);
            shader->setFloat("time", time);
            shader->setVec4("fogColor", r->getColor());
            shader->setFloat("density", g_FogDensity);
            shader->setFloat("noiseScale", g_FogNoiseScale);
            shader->setFloat("windSpeed", g_FogWindSpeed);
            shader->setFloat("noiseContrast", g_FogNoiseContrast);
            shader->setFloat("warpStrength", g_FogWarpStrength);

            mesh->draw();

            Profiler::Get().stats.drawCalls++;
            Profiler::Get().stats.renderedObjects++;
            Profiler::Get().stats.vertexCount += mesh->getVertexCount();
            Profiler::Get().stats.triangleCount += mesh->getVertexCount() / 3;
        }
    }

    // Restore saved OpenGL states
    glDepthMask(depthWriteEnabled);
    if (cullFaceEnabled) {
        glEnable(GL_CULL_FACE);
        glCullFace(cullFaceMode);
    } else {
        glDisable(GL_CULL_FACE);
    }

    if (blendEnabled) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
}
