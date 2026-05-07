#ifndef DZEMIKK_POINT_LIGHT_H
#define DZEMIKK_POINT_LIGHT_H

#include "ecs/component.h"
#include <glm/ext/vector_float3.hpp>

namespace dzemikk {
class PointLight : public Component {
  public:
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;

    float range = 10.0f; // attenuation radius

    bool castsShadows = false;

    std::string typeName() const override {
        return "PointLight";
    }
};
} // namespace dzemikk

#endif // DZEMIKK_POINT_LIGHT_H
