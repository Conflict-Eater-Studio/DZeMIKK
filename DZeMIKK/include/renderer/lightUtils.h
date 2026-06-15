#ifndef DZEMIKK_LIGHT_UTILS_H
#define DZEMIKK_LIGHT_UTILS_H

namespace dzemikk {
struct GPUDirectionalLight {
    glm::vec4 direction; // w xyz + padding
    glm::vec4 color;     // rgb + intensity
};

struct GPUPointLight {
    glm::vec4 position;
    glm::vec4 color;  // rgb + intensity
    glm::vec4 params; // x = range
};

struct GPUSpotLight {
    glm::vec4 position;
    glm::vec4 direction;
    glm::vec4 color;
    glm::vec4 params; // x = range, y = inner, z = outer
};

struct GPUSceneLights {
    int dirCount;
    int pointCount;
    int spotCount;
    int padding;

    GPUDirectionalLight dir[5000];
    GPUPointLight point[5000];
    GPUSpotLight spot[5000];
};
}

#endif // DZEMIKK_LIGHT_UTILS_H