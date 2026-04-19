#include "animation/skeleton.h"

int dzemikk::Skeleton::addBone(const std::string& name, int parentIndex) {
    auto it = _boneMap.find(name);
    if (it != _boneMap.end()) {
        return it->second;
    }

    int index = static_cast<int>(_bones.size());

    _bones.emplace_back(name, parentIndex);
    _boneMap[name] = index;

    if (parentIndex >= 0 && parentIndex < static_cast<int>(_bones.size())) {
        _bones[parentIndex].addChild(index);
    }

    return index;
}

int dzemikk::Skeleton::getBoneIndex(const std::string& name) const {
    auto it = _boneMap.find(name);
    if (it == _boneMap.end()) {
        return -1;
    }
    return it->second;
}

const dzemikk::Bone* dzemikk::Skeleton::getBone(std::size_t index) const {
    if (index >= _bones.size()) {
        return nullptr;
    }
    return &_bones[index];
}

std::size_t dzemikk::Skeleton::getBoneCount() const {
    return _bones.size();
}

const std::vector<dzemikk::Bone>& dzemikk::Skeleton::getBones() const {
    return _bones;
}

const glm::mat4& dzemikk::Skeleton::getGlobalInverseTransform() const {
    return _globalInverseTransform;
}

void dzemikk::Skeleton::setGlobalInverseTransform(const glm::mat4& transform) {
    _globalInverseTransform = transform;
}

dzemikk::Bone* dzemikk::Skeleton::getBone(int index) {
    return &_bones[index];
}