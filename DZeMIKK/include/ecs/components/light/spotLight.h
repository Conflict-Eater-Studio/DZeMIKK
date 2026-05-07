#ifndef DZEMIKK_SPOT_LIGHT_H
#define DZEMIKK_SPOT_LIGHT_H

#include "ecs/component.h"
#include <glm/ext/vector_float3.hpp>

namespace dzemikk {
class SpotLight : public Component {
  public:
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);

    float innerCutoff = 12.5f; // degrees
    float outerCutoff = 17.5f;

    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;

    float range = 15.0f;

    bool castsShadows = false;

    std::string typeName() const override {
        return "SpotLight";
    }
};
} // namespace dzemikk
#endif // DZEMIKK_SPOT_LIGHT_H
