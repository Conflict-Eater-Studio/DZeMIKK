#pragma once
#include <array>
#include <glm/glm.hpp>

namespace dzemikk {

struct Plane {
    glm::vec3 normal;
    float distance;

    inline float getDistanceToPoint(const glm::vec3& p) const {
        return glm::dot(normal, p) + distance;
    }
};

class Frustum {
  public:
    enum { Left = 0, Right, Bottom, Top, Near, Far };

    void update(const glm::mat4& vp) {
        extractPlanes(vp);
    }

    inline bool isSphereVisible(const glm::vec3& center, float radius) const {
        const Plane* p = planes.data();

        for (int i = 0; i < 6; ++i) {
            if (glm::dot(p[i].normal, center) + p[i].distance < -radius)
                return false;
        }
        return true;
    }

  private:
    std::array<Plane, 6> planes;

    void extractPlanes(const glm::mat4& vp) {
        // Left
        planes[Left].normal = {vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0]};
        planes[Left].distance = vp[3][3] + vp[3][0];

        // Right
        planes[Right].normal = {vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0]};
        planes[Right].distance = vp[3][3] - vp[3][0];

        // Bottom
        planes[Bottom].normal = {vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1]};
        planes[Bottom].distance = vp[3][3] + vp[3][1];

        // Top
        planes[Top].normal = {vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1]};
        planes[Top].distance = vp[3][3] - vp[3][1];

        // Near
        planes[Near].normal = {vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2]};
        planes[Near].distance = vp[3][3] + vp[3][2];

        // Far
        planes[Far].normal = {vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2]};
        planes[Far].distance = vp[3][3] - vp[3][2];

        for (int i = 0; i < 6; ++i) {
            float len = glm::length(planes[i].normal);
            planes[i].normal /= len;
            planes[i].distance /= len;
        }
    }
};

} // namespace dzemikk