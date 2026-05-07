#ifndef DZEMIKK_DIRECTIONAL_LIGHT_H
#define DZEMIKK_DIRECTIONAL_LIGHT_H

#include "ecs/component.h"
#include <glm/ext/vector_float3.hpp>

namespace dzemikk {
class DirectionalLight : public Component {
  public:
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;

    bool castsShadows = false;

    std::string typeName() const override {
        return "DirectionalLight";
    }
};

}

#endif // DZEMIKK_DIRECTIONAL_LIGHT_H