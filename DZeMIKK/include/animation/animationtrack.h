#ifndef DZEMIKK_ANIMATIONTRACK_H
#define DZEMIKK_ANIMATIONTRACK_H
#pragma once

#include "ecs/components/transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace dzemikk {

struct PositionKey {
    float time;
    glm::vec3 value;
};

struct RotationKey {
    float time;
    glm::quat value;
};

struct ScaleKey {
    float time;
    glm::vec3 value;
};

class AnimationTrack {
public:
    AnimationTrack(std::string name);

    const std::string& getName() const noexcept;

    void addPositionKey(float time, const glm::vec3& value);
    void addRotationKey(float time, const glm::quat& value);
    void addScaleKey(float time, const glm::vec3& value);

    glm::vec3 interpolatePosition(float time) const;
    glm::quat interpolateRotation(float time) const;
    glm::vec3 interpolateScale(float time) const;

    void setTransform(Transform* transform);
    Transform* getTransform() const;

private:
    std::string _name;
    Transform* _transform;
    std::vector<PositionKey> _positions;
    std::vector<RotationKey> _rotations;
    std::vector<ScaleKey> _scales;

    size_t findPositionIndex(float time) const;
    size_t findRotationIndex(float time) const;
    size_t findScaleIndex(float time) const;
};

}
#endif
