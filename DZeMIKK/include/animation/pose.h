#ifndef DZEMIKK_KEYFRAME_H
#define DZEMIKK_KEYFRAME_H

namespace dzemikk {
    class Transform;
    class Pose {
    public:
        ~Pose() = default;
        Pose() = default;
        Pose(Transform* transform); // Transform in local space
        Transform* transform;
    };
};
#endif
