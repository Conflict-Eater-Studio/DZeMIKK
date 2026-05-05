#include "ecs/components/camera.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"

dzemikk::Camera::Camera() {
    static int _nextId = 0;
    _id = _nextId++;

    setPerspective(DefaultFov, DefaultAspect, DefaultNear, DefaultFar);
}
dzemikk::Camera::ProjectionType dzemikk::Camera::getProjectionType() const {
    return _projectionType;
}
float dzemikk::Camera::getNear() const {
    return _near;
}
float dzemikk::Camera::getFar() const {
    return _far;
}
float dzemikk::Camera::getFov() const {
    return _fov;
}
float dzemikk::Camera::getAspect() const {
    return _aspect;
}
float dzemikk::Camera::getLeft() const {
    return _left;
}
float dzemikk::Camera::getBottom() const {
    return _bottom;
}
float dzemikk::Camera::getRightOrtographic() const {
    return _right;
}
float dzemikk::Camera::getTop() const {
    return _top;
}

void dzemikk::Camera::setPerspective(float fov, float aspect, float nearPlane, float farPlane) {
    _projectionType = ProjectionType::Perspective;
    _fov = fov;
    _aspect = aspect;
    _near = nearPlane;
    _far = farPlane;
    recalcProjection();
}

void dzemikk::Camera::setOrthographic(float left, float right, float bottom, float top,
                                      float nearPlane, float farPlane) {
    _projectionType = ProjectionType::Orthographic;
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _near = nearPlane;
    _far = farPlane;
    recalcProjection();
}

void dzemikk::Camera::recalcProjection() const {
    if (_projectionType == ProjectionType::Perspective) {
        _projection = glm::perspective(glm::radians(_fov), _aspect, _near, _far);
    } else {
        _projection = glm::ortho(_left, _right, _bottom, _top, _near, _far);
    }

    _viewProjectionDirty = true;
}

const glm::mat4& dzemikk::Camera::getProjection() const {
    return _projection;
}

const glm::mat4& dzemikk::Camera::getView() const {
    if (_projectionType == ProjectionType::Orthographic) {
        _view = glm::mat4(1.0f);
        return _view;
    }

    const Transform& t = *_owner->transform();
    const glm::mat4 currentTransform = t.getWorldMatrix();
    if (currentTransform != _lastTransform) {
        _viewDirty = true;
    }

    if (_viewDirty) {
        const glm::vec3 position = t.getPosition();
        const glm::quat rotation = glm::normalize(t.getRotation());
        const glm::vec3 forward = rotation * glm::vec3(0, 0, -1);
        const glm::vec3 up = rotation * glm::vec3(0, 1, 0);
        _view = glm::lookAt(position, position + forward, up);
        _lastTransform = currentTransform;
        _viewDirty = false;
        _viewProjectionDirty = true;
    }

    return _view;
}

 const glm::mat4& dzemikk::Camera::getViewProjection() const {
    if (_viewProjectionDirty) {
        _viewProjection = _projection * getView();
        _viewProjectionDirty = false;
    }

    return _viewProjection;
}

glm::vec3 dzemikk::Camera::getForward() const {
    const Transform& t = *_owner->transform();

    glm::vec3 forward = t.getRotation() * glm::vec3(0, 0, -1);

    return glm::normalize(forward);
}

glm::vec3 dzemikk::Camera::getRight() const {
    return glm::normalize(glm::cross(getForward(), glm::vec3(0, 1, 0)));
}

glm::vec3 dzemikk::Camera::getUp() const {
    return glm::normalize(glm::cross(getRight(), getForward()));
}

void dzemikk::Camera::lookAt(const glm::vec3& target) {
    Transform& t = *_owner->transform();

    glm::mat4 view = glm::lookAt(t.getPosition(), target, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::quat rotation = glm::quat_cast(glm::inverse(view));
    t.setRotation(rotation);

    _viewDirty = true;
    _viewProjectionDirty = true;
}

void dzemikk::Camera::setViewportSize(float width, float height) {
    _aspect = width / height;
    if (_projectionType == ProjectionType::Perspective)
        recalcProjection();
}

int dzemikk::Camera::getId() const {
    return _id;
}

