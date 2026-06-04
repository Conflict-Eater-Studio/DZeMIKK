#include "renderer/renderPasses/skyboxRenderPass.h"
#include "core/profiler.h"
#include <GLFW/glfw3.h>

void dzemikk::SkyboxRenderPass::execute(RenderContext& ctx) {
    if (_skybox && ctx.sceneCamera) {

        if (_skybox.get()->gpuReady) {
            float time = glfwGetTime();

            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), time * 0.1f, glm::vec3(0, 1, 0));
            glm::mat4 viewNoTrans = ctx.sceneCamera->getView() * rotation;

            _skybox.get()->render(ctx.sceneCamera->getView(), ctx.sceneCamera->getProjection());

            Profiler::Get().stats.drawCalls++;
            Profiler::Get().stats.renderedObjects++;
            Profiler::Get().stats.vertexCount += 36;
            Profiler::Get().stats.triangleCount += 12;
        }
    }
}

void dzemikk::SkyboxRenderPass::setSkybox(AssetHandle<Skybox> skybox) {
    _skybox = skybox;
}

const dzemikk::AssetHandle<dzemikk::Skybox> dzemikk::SkyboxRenderPass::getSkybox() const {
    return _skybox;
}
