#include "animation/pose.h"

namespace dzemikk {

Pose::Pose(const Transform& transform) {
    Pose::transform = transform;
}
} // namespace dzemikk