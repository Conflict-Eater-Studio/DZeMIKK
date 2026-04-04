#ifndef DZEMIKK_KEYFRAME_H
#define DZEMIKK_KEYFRAME_H
#include "ecs/components/transform.h"

namespace dzemikk {
class Pose {
    public:
        ~Pose() = default;
        Pose() = default;
        Pose(const Transform& transform);
        Transform transform;
    };
};
#endif
