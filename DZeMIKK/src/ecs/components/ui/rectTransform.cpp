#include "ecs/components/ui/rectTransform.h"

#include "ecs/gameobject.h"
#include "glm/fwd.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace dzemikk {
RectTransform::RectTransform(RectTransformParams params)
    : _position(params.position), _size(params.size), _scale(params.scale),
      _rotation(params.rotation), _pivot(params.pivot), _anchorMin(params.anchorMin),
      _anchorMax(params.anchorMax), _offsetMin(params.offsetMin), _offsetMax(params.offsetMax) {}

void RectTransform::setPosition(const glm::vec2& position) {
    _position = position;
    _localDirty = true;
    markDirty();
}

void RectTransform::setSize(const glm::vec2& size) {
    _size = size;
    _localDirty = true;
    markDirty();
    markSizeDirty();
}

void RectTransform::setScale(const glm::vec2& scale) {
    _scale = scale;
    _localDirty = true;
    markDirty();
}

void RectTransform::setRotation(float rotation) {
    _rotation = rotation;
    _localDirty = true;
    markDirty();
}

void RectTransform::setPivot(const glm::vec2& pivot) {
    _pivot = pivot;
    _localDirty = true;
    markDirty();
}

void RectTransform::setAnchorMin(const glm::vec2& anchorMin) {
    _anchorMin = anchorMin;
    _localDirty = true;
    markDirty();
    markSizeDirty();
}

void RectTransform::setAnchorMax(const glm::vec2& anchorMax) {
    _anchorMax = anchorMax;
    _localDirty = true;
    markDirty();
    markSizeDirty();
}

void RectTransform::setOffsetMin(const glm::vec2& offsetMin) {
    _offsetMin = offsetMin;
    _localDirty = true;
    markDirty();
    markSizeDirty();
}

void RectTransform::setOffsetMax(const glm::vec2& offsetMax) {
    _offsetMax = offsetMax;
    _localDirty = true;
    markDirty();
    markSizeDirty();
}

void RectTransform::setZIndex(unsigned int zIndex) {
    _zIndex = zIndex;
    markDirty();
}

void RectTransform::translate(const glm::vec2& delta) {
    setPosition(_position + delta);
}

void RectTransform::rotate(float deltaDegrees) {
    setRotation(_rotation + deltaDegrees);
}

void RectTransform::scale(const glm::vec2& delta) {
    setScale(_scale + delta);
}

void RectTransform::scale(float uniform) {
    setScale(_scale + glm::vec2(uniform));
}

glm::vec2 RectTransform::getPosition() const {
    return _position;
}

glm::vec2 RectTransform::getSize() const {
    if (_sizeDirty) {
        _cachedSize = _size + getStretchSize();
        _sizeDirty = false;
    }
    return _cachedSize;
}

glm::vec2 RectTransform::getScale() const {
    return _scale;
}

float RectTransform::getRotation() const {
    return _rotation;
}

glm::vec2 RectTransform::getPivot() const {
    return _pivot;
}

glm::vec2 RectTransform::getAnchorMin() const {
    return _anchorMin;
}

glm::vec2 RectTransform::getAnchorMax() const {
    return _anchorMax;
}

glm::vec2 RectTransform::getOffsetMin() const {
    return _offsetMin;
}

glm::vec2 RectTransform::getOffsetMax() const {
    return _offsetMax;
}

unsigned int RectTransform::getZIndex() const {
    return _zIndex;
}

glm::mat4 RectTransform::getLocalMatrix() const {
    if (_localDirty) {
        _cachedLocalNoSizeMatrix = getLocalNoSizeMatrix();
        _cachedLocalSizeMatrix = getLocalSizeMatrix();
        _cachedLocalMatrix = _cachedLocalNoSizeMatrix * _cachedLocalSizeMatrix;
        _localDirty = false;
    }
    return _cachedLocalMatrix;
}

glm::mat4 RectTransform::getWorldMatrix() const {
    if (_worldDirty) {
        _cachedWorldNoSizeMatrix = getWorldNoSizeMatrix();
        _cachedWorldMatrix = _cachedWorldNoSizeMatrix * getLocalSizeMatrix();
        _worldDirty = false;
    }
    return _cachedWorldMatrix;
}

glm::mat4 RectTransform::getLocalNoSizeMatrix() const {
    const GameObject* parent = _owner ? _owner->getParent() : nullptr;
    const RectTransform* parentRect = parent ? parent->getComponent<RectTransform>() : nullptr;

    glm::vec2 anchorCenter(0.0F);
    if (parentRect) {
        const glm::vec2 parentSize = parentRect->getSize();
        const glm::vec2 parentPivot = parentRect->getPivot();
        const glm::vec2 anchorNormalized = (_anchorMin + _anchorMax) * 0.5F;
        anchorCenter = (anchorNormalized - parentPivot) * parentSize;
    } else {
        const glm::vec2 anchorNormalized = (_anchorMin + _anchorMax) * 0.5F;
        anchorCenter = anchorNormalized * getSize();
    }

    glm::mat4 translation =
        glm::translate(glm::mat4(1.0F), glm::vec3(anchorCenter + _position, 0.0F));
    glm::mat4 rotation =
        glm::rotate(glm::mat4(1.0F), glm::radians(_rotation), glm::vec3(0.0F, 0.0F, 1.0F));
    glm::mat4 scale = glm::scale(glm::mat4(1.0F), glm::vec3(_scale, 1.0F));

    return translation * rotation * scale;
}

glm::mat4 RectTransform::getLocalSizeMatrix() const {
    const glm::vec2 finalSize = getSize();
    const glm::vec2 pivotOffset = -_pivot * finalSize;

    glm::mat4 pivotTranslation = glm::translate(glm::mat4(1.0F), glm::vec3(pivotOffset, 0.0F));
    glm::mat4 sizeScale = glm::scale(glm::mat4(1.0F), glm::vec3(finalSize, 1.0F));

    return pivotTranslation * sizeScale;
}

glm::mat4 RectTransform::getWorldNoSizeMatrix() const {
    if (_localDirty) {
        _cachedLocalNoSizeMatrix = getLocalNoSizeMatrix();
        _cachedLocalSizeMatrix = getLocalSizeMatrix();
        _cachedLocalMatrix = _cachedLocalNoSizeMatrix * _cachedLocalSizeMatrix;
        _localDirty = false;
    }

    const GameObject* parent = _owner ? _owner->getParent() : nullptr;
    const RectTransform* parentRect = parent ? parent->getComponent<RectTransform>() : nullptr;

    if (parentRect) {
        return parentRect->getWorldNoSizeMatrix() * _cachedLocalNoSizeMatrix;
    }

    return _cachedLocalNoSizeMatrix;
}

void RectTransform::markDirty() {
    if (_worldDirty && _localDirty) {
        return;
    }

    _localDirty = true;
    _worldDirty = true;

    if (!_owner) {
        return;
    }

    for (auto* child : _owner->getChildren()) {
        if (auto* childRect = child->getComponent<RectTransform>()) {
            childRect->markDirty();
        }
    }
}

glm::vec2 RectTransform::getStretchSize() const {
    const GameObject* parent = _owner ? _owner->getParent() : nullptr;
    const RectTransform* parentRect = parent ? parent->getComponent<RectTransform>() : nullptr;
    if (!parentRect) {
        return glm::vec2(0.0F);
    }

    const glm::vec2 parentSize = parentRect->getSize();
    const glm::vec2 anchorSpan = glm::max(_anchorMax - _anchorMin, glm::vec2(0.0F));
    return anchorSpan * parentSize - _offsetMin - _offsetMax;
}

void RectTransform::markSizeDirty() {
    if (_sizeDirty) {
        return;
    }

    _sizeDirty = true;
    _localDirty = true;
    _worldDirty = true;

    if (!_owner) {
        return;
    }

    for (auto* child : _owner->getChildren()) {
        if (auto* childRect = child->getComponent<RectTransform>()) {
            childRect->markSizeDirty();
        }
    }
}

bool RectTransform::containsPoint(const glm::vec2& point) const {
    glm::vec4 localPos = glm::inverse(getWorldMatrix()) * glm::vec4(point, 0.0F, 1.0F);

    glm::vec2 p = glm::vec2(localPos) / localPos[3];

    return p[0] >= 0.0F && p[0] <= 1.0F && p[1] >= 0.0F && p[1] <= 1.0F;
}
} // namespace dzemikk
