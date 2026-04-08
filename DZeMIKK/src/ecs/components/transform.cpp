#define GLM_ENABLE_EXPERIMENTAL
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace dzemikk {
    // --- Constructors & Destructor
    Transform::Transform(TransformParams params)
    : _position(params.position)
    , _rotation(glm::quat(glm::radians(params.rotation)))
    , _scale(params.scale)
    {}

    // --- Setters
    void Transform::setPosition(const glm::vec3& position) {
        _position = position;
        _localDirty = true;
        markDirty();
    }

    void Transform::setRotation(const glm::quat& rotation) {
        _rotation = rotation;
        _localDirty = true;
        markDirty();
    }

    void Transform::setScale(const glm::vec3& scale) {
        _scale = scale;
        _localDirty = true;
        markDirty();
    }

    void Transform::setEulerAngles(const glm::vec3& rotation) {
        _rotation = glm::quat(glm::radians(rotation));
        _localDirty = true;
        markDirty();
    }

    // --- Getters
    glm::vec3 Transform::getPosition() const {
        return _position;
    }

    glm::quat Transform::getRotation() const {
        return _rotation;
    }

    glm::vec3 Transform::getScale() const {
        return _scale;
    }

    glm::vec3 Transform::getEulerAngles() const {
        return glm::degrees(glm::eulerAngles(_rotation));
    }

    // --- Modifiers
    void Transform::translate(const glm::vec3& delta) {
        _position += delta;
        _localDirty = true;
        markDirty();
    }

    void Transform::rotate(const glm::quat& quat) {
        _rotation = quat * _rotation;
        _localDirty = true;
        markDirty();
    }

    void Transform::rotateEuler(const glm::vec3& degrees) {
        rotate(glm::quat(glm::radians(degrees)));
    }

    void Transform::rotateAround(float degrees, const glm::vec3& axis) {
        rotate(glm::angleAxis(glm::radians(degrees), axis));
    }

    void Transform::scale(const glm::vec3& scale) {
        _scale *= scale;
        _localDirty = true;
        markDirty();
    }

    void Transform::scale(float uniform) {
        scale(glm::vec3(uniform));
    }

    // --- Direction vectors
    glm::vec3 Transform::forward() const {
        return _rotation * glm::vec3(0.0F, 0.0F, -1.0F);
    }

    glm::vec3 Transform::right() const {
        return _rotation * glm::vec3(1.0F, 0.0F, 0.0F);
    }

    glm::vec3 Transform::up() const {
        return _rotation * glm::vec3(0.0F, 1.0F, 0.0F);
    }

    // --- Matrix
    const glm::mat4& Transform::getLocalMatrix() const {
        if (_localDirty) {
            glm::mat4 translation = glm::translate(glm::mat4(1.0F), _position);
            glm::mat4 rotation = glm::toMat4(_rotation);
            glm::mat4 scale = glm::scale(glm::mat4(1.0F), _scale);
            _cachedLocalMatrix = translation * rotation * scale;
            _localDirty = false;
        }
        return _cachedLocalMatrix;
    }

    glm::mat4 Transform::getWorldMatrix() const {
        if (_worldDirty) {
            if (_owner && _owner->getParent()) {
                _cachedWorldMatrix = _owner->getParent()->transform()->getWorldMatrix() * getLocalMatrix();
            } else {
                _cachedWorldMatrix = getLocalMatrix();
            }
            _worldDirty = false;
        }
        return _cachedWorldMatrix;
    }
    
    void Transform::markDirty() {
        if (_worldDirty) {
            return;
        }

        _worldDirty = true;

        for (auto* child : _owner->getChildren()) {
            child->transform()->markDirty();
        }
    }
}