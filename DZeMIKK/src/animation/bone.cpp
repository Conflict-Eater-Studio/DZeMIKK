#include "animation/bone.h"

dzemikk::Bone::Bone(std::string name, int parentIndex)
    : _name(std::move(name)), _parentIndex(parentIndex), _offsetMatrix(1.0F),
      _localTransform(1.0F) {}

const std::string& dzemikk::Bone::getName() const {
    return _name;
}

int dzemikk::Bone::getParentIndex() const {
    return _parentIndex;
}

const glm::mat4& dzemikk::Bone::getOffsetMatrix() const {
    return _offsetMatrix;
}

const glm::mat4& dzemikk::Bone::getLocalTransform() const {
    return _localTransform;
}

const glm::mat4& dzemikk::Bone::getBindLocalTransform() const {
    return _bindLocalTransform;
}

const std::vector<int>& dzemikk::Bone::getChildren() const {
    return _children;
}

void dzemikk::Bone::setOffsetMatrix(const glm::mat4& offset) {
    _offsetMatrix = offset;
}

void dzemikk::Bone::setParent(int parent) {
    _parentIndex = parent;
}

void dzemikk::Bone::setLocalTransform(const glm::mat4& transform) {
    _localTransform = transform;
}

void dzemikk::Bone::setBindLocalTransform(const glm::mat4& transform) {
    _bindLocalTransform = transform;
}

void dzemikk::Bone::addChild(int childIndex) {
    _children.push_back(childIndex);
}
