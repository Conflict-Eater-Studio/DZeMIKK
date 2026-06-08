#pragma once
#ifndef DZEMIKK_ROOTMOTION_H
#define DZEMIKK_ROOTMOTION_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace dzemikk {

enum class RootMotionMode : uint8_t {
    None,
    Position,
    Rotation,
    PositionAndRotation
};

struct RootMotionDelta {
    glm::vec3 deltaPosition{0.0f};
    glm::quat deltaRotation{1.0f, 0.0f, 0.0f, 0.0f};
};

} // namespace dzemikk

#endif
