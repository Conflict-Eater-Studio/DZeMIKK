#ifndef DZEMIKK_CAMERA_H
#define DZEMIKK_CAMERA_H

#include "ecs/component.h"

#include <glm/glm.hpp>


namespace dzemikk {
/**
 * @brief Encapsulates a 3D camera with view and projection matrices.
 *
 * Provides utilities to set perspective or orthographic projections,
 * compute view and view-projection matrices, and query direction vectors.
 */
class Camera : public Component {
  public:
    using Base = Component;
    /**
     * @brief Types of projection the camera can use.
     */
    enum class ProjectionType { Perspective, Orthographic };

    // --- Default constants
    static constexpr float DefaultFov = 60.0f;
    static constexpr float DefaultAspect = 1.77f;
    static constexpr float DefaultNear = 0.1f;
    static constexpr float DefaultFar = 1000.0f;

    /**
     * @brief Constructs a new Camera object with a unique ID and default perspective.
     */
    Camera();
    ~Camera() = default;

    ProjectionType getProjectionType() const;
    float getNear() const;
    float getFar() const;
    float getFov() const;
    float getAspect() const;
    float getLeft() const;
    float getBottom() const;
    float getRightOrtographic() const;
    float getTop() const;

#pragma region Disable copy and move

    Camera(const Camera& other) = delete;
    Camera(Camera&& other) noexcept = delete;
    Camera& operator=(const Camera& other) = delete;
    Camera& operator=(Camera&& other) noexcept = delete;

#pragma endregion

#pragma region Projection setup

    /**
     * @brief Sets the camera to use a perspective projection.
     *
     * @param fov Field of view in degrees.
     * @param aspect Aspect ratio (width / height).
     * @param nearPlane Near clipping plane distance.
     * @param farPlane Far clipping plane distance.
     */
    void setPerspective(float fov, float aspect, float nearPlane, float farPlane);

    /**
     * @brief Sets the camera to use an orthographic projection.
     *
     * @param left Left boundary of the orthographic view.
     * @param right Right boundary of the orthographic view.
     * @param bottom Bottom boundary of the orthographic view.
     * @param top Top boundary of the orthographic view.
     * @param nearPlane Near clipping plane distance.
     * @param farPlane Far clipping plane distance.
     */
    void setOrthographic(float left, float right, float bottom, float top, float nearPlane,
                         float farPlane);

    /**
     * @brief Updates the camera aspect ratio based on viewport size.
     *
     * @param width Width of the viewport.
     * @param height Height of the viewport.
     */
    void setViewportSize(float width, float height);

#pragma endregion

#pragma region Matrices

    /**
     * @brief Returns the current projection matrix of the camera.
     *
     * @return const glm::mat4& The projection matrix.
     */
    [[nodiscard]] const glm::mat4& getProjection() const;

    /**
     * @brief Returns the current view matrix of the camera.
     *
     * @return const glm::mat4& The view matrix.
     */
    [[nodiscard]] const glm::mat4& getView() const;

    /**
     * @brief Returns the combined view-projection matrix.
     *
     * @return const glm::mat4& The view-projection matrix.
     */
    [[nodiscard]] const glm::mat4& getViewProjection() const;

#pragma endregion

#pragma region Direction helpers

    /**
     * @brief Returns the forward direction vector of the camera.
     *
     * @return glm::vec3 Normalized forward direction.
     */
    [[nodiscard]] glm::vec3 getForward() const;

    /**
     * @brief Returns the right direction vector of the camera.
     *
     * @return glm::vec3 Normalized right direction.
     */
    [[nodiscard]] glm::vec3 getRight() const;

    /**
     * @brief Returns the up direction vector of the camera.
     *
     * @return glm::vec3 Normalized up direction.
     */
    [[nodiscard]] glm::vec3 getUp() const;

#pragma endregion

#pragma region Look at setup

    /**
     * @brief Rotates the camera to look at a given target point.
     *
     * @param target The position to look at.
     */
    void lookAt(const glm::vec3& target);

#pragma endregion

#pragma region Id

    /**
     * @brief Returns the unique ID of this camera.
     *
     * @return int Unique camera identifier.
     */
    [[nodiscard]] int getId() const;

#pragma endregion

    bool isDirty() const {
        return _viewProjectionDirty;
    }

    [[nodiscard]] std::string typeName() const override {
        return "Camera";
    };

  private:
    int _id = -1;
    ProjectionType _projectionType = ProjectionType::Perspective;

#pragma region Perspective

    float _fov = DefaultFov;
    float _aspect = DefaultAspect;

#pragma endregion

#pragma region Shared

    float _near = DefaultNear;
    float _far = DefaultFar;

#pragma endregion

#pragma region Ortho

    float _left = -1.0f;
    float _right = 1.0f;
    float _bottom = -1.0f;
    float _top = 1.0f;

#pragma endregion

#pragma region Cached matrices

    mutable glm::mat4 _projection = glm::mat4(1.0f);
    mutable glm::mat4 _view = glm::mat4(1.0f);
    mutable glm::mat4 _viewProjection = glm::mat4(1.0f);
    mutable glm::mat4 _lastTransform = glm::mat4(1.0f);

#pragma endregion

#pragma region Dirty flags

    mutable bool _viewDirty = true;
    mutable bool _viewProjectionDirty = true;

#pragma endregion

#pragma region Methods

    void recalcProjection() const;

#pragma endregion
};
} // namespace dzemikk
#endif // DZEMIKK_CAMERA_H