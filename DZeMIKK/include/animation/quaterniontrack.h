#pragma once
#ifndef DZEMIKK_QUATERNIONTRACK_H
#define DZEMIKK_QUATERNIONTRACK_H
#define GLM_ENABLE_EXPERIMENTAL

#include "IAnimationTrack.h"
#include "property.h"
#include "propertykey.h"

#include <glm/gtx/quaternion.hpp>

namespace dzemikk {

using QuaternionPropertyKey = PropertyKey<glm::quat>;
using QuaternionProperty = Property<glm::quat>;

class QuaternionTrack : public IAnimationTrack {
    public:
        void apply(float time) override {
            if (_keys.empty()) {
                return;
            }
            glm::quat value = sample(time);
            _property.set(value);
        }
        void addKey(QuaternionPropertyKey key) {
                _keys.push_back(key);
        }
        void setProperty(const QuaternionProperty& property) {
            _property = property;
        }
        QuaternionProperty getProperty() const {
            return _property;
        }
    private:
        QuaternionProperty _property;
        std::vector<QuaternionPropertyKey> _keys;

        glm::quat sample(float time) const {
            if (_keys.size() == 1) {
                return _keys[0].value;
            }

            size_t index = findPropertyIndex(time);
            size_t next = index + 1;

            float t1 = _keys[index].time;
            float t2 = _keys[next].time;

            float factor = (time - t1) / (t2 - t1);

            glm::quat value = glm::slerp(_keys[index].value, _keys[next].value, factor);

            return value;
        }
        size_t findPropertyIndex(float time) const {
            for (size_t i = 0; i < _keys.size() - 1; i++) {
                if (time < _keys[i + 1].time)
                    return i;
            }
            return _keys.size() - 2;
        }
};
}
#endif
