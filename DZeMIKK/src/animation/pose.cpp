#include "animation/pose.h"

namespace dzemikk {

Pose::Pose(Transform* transform) {
    Pose::transform = transform;
}
} // namespace dzemikk