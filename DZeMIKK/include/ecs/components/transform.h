#ifndef DZEMIKK_TRANSFORM_H
#define DZEMIKK_TRANSFORM_H

#include "../component.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace dzemikk {
class GameObject;
struct TransformParams {
    glm::vec3 position = glm::vec3(0.0F);
    glm::vec3 rotation = glm::vec3(0.0F);
    glm::vec3 scale = glm::vec3(1.0F);
};

class Transform : public Component {
  public:
    using Base = Component;

    // --- Constructors & Destructor
    Transform(TransformParams params = TransformParams());
    Transform(const Transform& other) = delete;
    Transform& operator=(const Transform& other) = delete;
    Transform(Transform&& other) noexcept = delete;
    Transform operator=(Transform&& other) noexcept = delete;
    ~Transform() override = default;

    // --- Setters
    void setPosition(const glm::vec3& position);
    void setRotation(const glm::quat& rotation);
    void setScale(const glm::vec3& scale);

    void setEulerAngles(const glm::vec3& rotation);

    // --- Getters
    [[nodiscard]] glm::vec3 getPosition() const;
    [[nodiscard]] glm::quat getRotation() const;
    [[nodiscard]] glm::vec3 getScale() const;

    [[nodiscard]] glm::vec3 getEulerAngles() const;

    // --- Modifiers
    void translate(const glm::vec3& delta);
    void rotate(const glm::quat& quat);
    void rotateEuler(const glm::vec3& degrees);
    void rotateAround(float degrees, const glm::vec3& axis);
    void scale(const glm::vec3& scale);
    void scale(float uniform);

    // --- Direction vectors
    [[nodiscard]] glm::vec3 forward() const;
    [[nodiscard]] glm::vec3 right() const;
    [[nodiscard]] glm::vec3 up() const;

    // --- Matrix
    const glm::mat4& getLocalMatrix() const;
    glm::mat4 getWorldMatrix() const;

    void markDirty();

  private:
    glm::vec3 _position = glm::vec3(0.0F);
    glm::quat _rotation = glm::quat(1.0F, 0.0F, 0.0F, 0.0F);
    glm::vec3 _scale = glm::vec3(1.0F);

    mutable glm::mat4 _cachedLocalMatrix = glm::mat4(1.0F);
    mutable glm::mat4 _cachedWorldMatrix = glm::mat4(1.0F);
    mutable bool _localDirty = true;
    mutable bool _worldDirty = true;
};
} // namespace dzemikk

#endif // DZEMIKK_TRANSFORM_H
