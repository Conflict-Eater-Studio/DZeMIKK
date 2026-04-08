#ifndef DZEMIKK_FRUSTUM_H
#define DZEMIKK_FRUSTUM_H

#include <array>
#include <glm/glm.hpp>

namespace dzemikk {

    /**
     * @brief Represents a plane in 3D space.
     *
     * Defined by a normalized normal vector and a distance from the origin.
     */
    struct Plane {

        /**
         * @brief Normal vector of the plane (should be normalized).
         */
        glm::vec3 normal = glm::vec3(0.0f);

        /**
         * @brief Distance from the origin.
         */
        float distance = 0.0f;

        /**
         * @brief Computes signed distance from the plane to a point.
         *
         * @param point Point in world space.
         * @return float Signed distance (negative = behind plane).
         */
        [[nodiscard]] float getDistanceToPoint(const glm::vec3& point) const {
            return glm::dot(normal, point) + distance;
        }
    };

    /**
     * @brief Represents a camera view frustum for visibility testing.
     *
     * Extracted from the view-projection matrix and used for frustum culling.
     */
    class Frustum {
    public:
        /**
         * @brief Indices of frustum planes.
         */
        enum class PlaneIndex { Left = 0, Right, Bottom, Top, Near, Far };

        /**
         * @brief Constructs an empty frustum.
         */
        Frustum() = default;
        ~Frustum() = default;

        #pragma region Disable copy and move

        Frustum(const Frustum& other) = default;
        Frustum(Frustum&& other) noexcept = default;
        Frustum& operator=(const Frustum& other) = default;
        Frustum& operator=(Frustum&& other) noexcept = default;

        #pragma endregion

        #pragma region Update

        /**
         * @brief Updates frustum planes from a view-projection matrix.
         *
         * @param viewProjection Combined view-projection matrix.
         */
        void update(const glm::mat4& viewProjection);

        #pragma endregion

        #pragma region Visibility tests

        /**
         * @brief Checks if a bounding sphere is visible inside the frustum.
         *
         * @param center Center of the sphere.
         * @param radius Radius of the sphere.
         * @return true If visible.
         * @return false If completely outside.
         */
        [[nodiscard]] bool isSphereVisible(const glm::vec3& center, float radius) const;

        #pragma endregion

        #pragma region Accessors

        /**
         * @brief Returns a specific frustum plane.
         *
         * @param index Plane index.
         * @return const Plane& Reference to plane.
         */
        [[nodiscard]] const Plane& getPlane(PlaneIndex index) const {
            return _planes[static_cast<int>(index)];
        }

        #pragma endregion

    private:
        #pragma region Data

        std::array<Plane, 6> _planes;

        #pragma endregion

        #pragma region Methods

        /**
         * @brief Extracts frustum planes from a view-projection matrix.
         *
         * @param viewProjection Combined matrix.
         */
        void extractPlanes(const glm::mat4& viewProjection);

        /**
         * @brief Normalizes all frustum planes.
         */
        void normalizePlanes();

        #pragma endregion
    };

} // namespace dzemikk
#endif // DZEMIKK_FRUSTUM_H