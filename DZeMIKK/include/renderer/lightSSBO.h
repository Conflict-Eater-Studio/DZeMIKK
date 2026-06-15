#ifndef DZEMIKK_LIGHT_SSBO_H
#define DZEMIKK_LIGHT_SSBO_H

#include "lightSystem.h"

namespace dzemikk {

class LightSSBO {
  public:
    /**
     * @brief Creates the GPU shader storage buffer used for light data.
     */
    void init() {
        glGenBuffers(1, &_ssbo);
    }

    /**
     * @brief Releases the GPU shader storage buffer.
     */
    void destroy() {
        glDeleteBuffers(1, &_ssbo);
    }

    /**
     * @brief Uploads all scene lights to the GPU.
     *
     * Copies directional, point, and spot light data from the
     * LightSystem into a GPUSceneLights structure and uploads it
     * to a Shader Storage Buffer Object (SSBO).
     *
     * @param lights Source light system containing scene light data.
     */
    void upload(const LightSystem& lights) {
        GPUSceneLights gpu;

        gpu.dirCount = lights.dirCount();
        gpu.pointCount = lights.pointCount();
        gpu.spotCount = lights.spotCount();

        memcpy(gpu.dir, lights.dirData(), sizeof(GPUDirectionalLight) * gpu.dirCount);

        memcpy(gpu.point, lights.pointData(), sizeof(GPUPointLight) * gpu.pointCount);

        memcpy(gpu.spot, lights.spotData(), sizeof(GPUSpotLight) * gpu.spotCount);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo);

        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUSceneLights), &gpu, GL_DYNAMIC_DRAW);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _ssbo);
    }

  private:
    /**
     * @brief OpenGL handle to the shader storage buffer object.
     *
     * Stores all scene light data accessible from shaders.
     */
    GLuint _ssbo = 0;
};

} // namespace dzemikk

#endif