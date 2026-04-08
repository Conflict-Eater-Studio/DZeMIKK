#include "renderer/frustum.h"

void dzemikk::Frustum::update(const glm::mat4& vp) {
    extractPlanes(vp);
    normalizePlanes();
}

bool dzemikk::Frustum::isSphereVisible(const glm::vec3& center, float radius) const {
    for (const Plane& plane : _planes) {
        if (plane.getDistanceToPoint(center) < -radius)
            return false;
    }
    return true;
}

void dzemikk::Frustum::extractPlanes(const glm::mat4& vp) {
    _planes[static_cast<int>(PlaneIndex::Left)].normal = {vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0]};
    _planes[static_cast<int>(PlaneIndex::Left)].distance = vp[3][3] + vp[3][0];

    _planes[static_cast<int>(PlaneIndex::Right)].normal = {vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0]};
    _planes[static_cast<int>(PlaneIndex::Right)].distance = vp[3][3] - vp[3][0];

    _planes[static_cast<int>(PlaneIndex::Bottom)].normal = {vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1]};
    _planes[static_cast<int>(PlaneIndex::Bottom)].distance = vp[3][3] + vp[3][1];

    _planes[static_cast<int>(PlaneIndex::Top)].normal = {vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1]};
    _planes[static_cast<int>(PlaneIndex::Top)].distance = vp[3][3] - vp[3][1];

    _planes[static_cast<int>(PlaneIndex::Near)].normal = {vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2]};
    _planes[static_cast<int>(PlaneIndex::Near)].distance = vp[3][3] + vp[3][2];


    _planes[static_cast<int>(PlaneIndex::Far)].normal = {vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2]};
    _planes[static_cast<int>(PlaneIndex::Far)].distance = vp[3][3] - vp[3][2];
}

void dzemikk::Frustum::normalizePlanes() {
    for (Plane& plane : _planes) {
        float length = glm::length(plane.normal);
        plane.normal /= length;
        plane.distance /= length;
    }
}