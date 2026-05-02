#ifndef TUL_PBL_DZEMIKK_BONETRACK_H
#define TUL_PBL_DZEMIKK_BONETRACK_H

#define GLM_ENABLE_EXPERIMENTAL

#include "IAnimationTrack.h"
#include "animation/skeleton.h"
#include "propertykey.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

namespace dzemikk {

using BonePositionKey = PropertyKey<glm::vec3>;
using BoneRotationKey = PropertyKey<glm::quat>;
using BoneScaleKey = PropertyKey<glm::vec3>;

class BoneTrack : public IAnimationTrack {
  public:
    void apply(float time) override {
        const glm::vec3 position = samplePosition(time);
        const glm::quat rotation = sampleRotation(time);
        const glm::vec3 scale = sampleScale(time);

        const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0F), position);
        const glm::mat4 rotationMatrix = glm::toMat4(rotation);
        const glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0F), scale);

        if (_skeleton && _boneIndex >= 0) {
            Bone* bone = _skeleton->getBone(_boneIndex);
            if (bone) {
                bone->setLocalTransform(translationMatrix * rotationMatrix * scaleMatrix);
            }
        }
    }

    void bindBone(Skeleton* skeleton, int index) {
        _skeleton = skeleton;
        _boneIndex = index;
    }

    void addPositionKey(const BonePositionKey& key) {
        _positionKeys.push_back(key);
    }

    void addRotationKey(const BoneRotationKey& key) {
        _rotationKeys.push_back(key);
    }

    void addScaleKey(const BoneScaleKey& key) {
        _scaleKeys.push_back(key);
    }

    [[nodiscard]] int getBone() const {
        return _boneIndex;
    }

    [[nodiscard]] const std::vector<BonePositionKey>& getPositionKeys() const {
        return _positionKeys;
    }

    [[nodiscard]] const std::vector<BoneRotationKey>& getRotationKeys() const {
        return _rotationKeys;
    }

    [[nodiscard]] const std::vector<BoneScaleKey>& getScaleKeys() const {
        return _scaleKeys;
    }

  private:
    int _boneIndex = -1;
    Skeleton* _skeleton = nullptr; 

    std::vector<BonePositionKey> _positionKeys;
    std::vector<BoneRotationKey> _rotationKeys;
    std::vector<BoneScaleKey> _scaleKeys;

    [[nodiscard]] glm::vec3 samplePosition(float time) const {
        if (_positionKeys.empty()) {
            return glm::vec3(0.0F);
        }

        if (_positionKeys.size() == 1) {
            return _positionKeys[0].value;
        }

        const size_t index = findPositionIndex(time);
        const size_t next = index + 1;

        const float t1 = _positionKeys[index].time;
        const float t2 = _positionKeys[next].time;
        const float factor = (time - t1) / (t2 - t1);

        return glm::mix(_positionKeys[index].value, _positionKeys[next].value, factor);
    }

    [[nodiscard]] glm::quat sampleRotation(float time) const {
        if (_rotationKeys.empty()) {
            return glm::quat(1.0F, 0.0F, 0.0F, 0.0F);
        }

        if (_rotationKeys.size() == 1) {
            return _rotationKeys[0].value;
        }

        const size_t index = findRotationIndex(time);
        const size_t next = index + 1;

        const float t1 = _rotationKeys[index].time;
        const float t2 = _rotationKeys[next].time;
        const float factor = (time - t1) / (t2 - t1);

        return glm::normalize(
            glm::slerp(_rotationKeys[index].value, _rotationKeys[next].value, factor));
    }

    [[nodiscard]] glm::vec3 sampleScale(float time) const {
        if (_scaleKeys.empty()) {
            return glm::vec3(1.0F);
        }

        if (_scaleKeys.size() == 1) {
            return _scaleKeys[0].value;
        }

        const size_t index = findScaleIndex(time);
        const size_t next = index + 1;

        const float t1 = _scaleKeys[index].time;
        const float t2 = _scaleKeys[next].time;
        const float factor = (time - t1) / (t2 - t1);

        return glm::mix(_scaleKeys[index].value, _scaleKeys[next].value, factor);
    }

    [[nodiscard]] size_t findPositionIndex(float time) const {
        for (size_t i = 0; i < _positionKeys.size() - 1; i++) {
            if (time < _positionKeys[i + 1].time) {
                return i;
            }
        }

        return _positionKeys.size() - 2;
    }

    [[nodiscard]] size_t findRotationIndex(float time) const {
        for (size_t i = 0; i < _rotationKeys.size() - 1; i++) {
            if (time < _rotationKeys[i + 1].time) {
                return i;
            }
        }

        return _rotationKeys.size() - 2;
    }

    [[nodiscard]] size_t findScaleIndex(float time) const {
        for (size_t i = 0; i < _scaleKeys.size() - 1; i++) {
            if (time < _scaleKeys[i + 1].time) {
                return i;
            }
        }

        return _scaleKeys.size() - 2;
    }
};

} // namespace dzemikk

#endif // TUL_PBL_DZEMIKK_BONETRACK_H