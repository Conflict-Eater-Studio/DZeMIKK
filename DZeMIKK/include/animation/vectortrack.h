#ifndef DZEMIKK_VECTORTRACK_H
#define DZEMIKK_VECTORTRACK_H

#include "IAnimationTrack.h"
#include "glm/ext/quaternion_common.hpp"
#include "glm/vec3.hpp"

#include <functional>

namespace dzemikk {

struct VectorPropertyKey {
    float time;
    glm::vec3 value;
};

class VectorProperty {
public:
    using Getter = std::function<glm::vec3()>;
    using Setter = std::function<void(glm::vec3)>;

    VectorProperty() = default;
    VectorProperty(Getter g, Setter s)
        : _get(std::move(g)), _set(std::move(s)) {}

    void set(glm::vec3 value) {
        _set(value);
    }

    glm::vec3 get() const {
        return _get();
    }

private:
    Getter _get;
    Setter _set;
};

class VectorTrack : public IAnimationTrack {
    public:
        void apply(float time) override {
            if (_keys.empty()) {
                return;
            }
            glm::vec3 value = sample(time);
            _property.set(value);
        }
        void addKey(VectorPropertyKey key) {
                _keys.push_back(key);
        }
        void setProperty(const VectorProperty& property) {
            _property = property;
        }
        VectorProperty getProperty() const {
            return _property;
        }
    private:
        VectorProperty _property;
        std::vector<VectorPropertyKey> _keys;

        glm::vec3 sample(float time) const {
            if (_keys.size() == 1) {
                return _keys[0].value;
            }

            size_t index = findPropertyIndex(time);
            size_t next = index + 1;

            float t1 = _keys[index].time;
            float t2 = _keys[next].time;

            float factor = (time - t1) / (t2 - t1);

            float x = glm::mix(_keys[index].value.x,
                _keys[next].value.x,
                factor);
            float y = glm::mix(_keys[index].value.y,
                _keys[next].value.y,
                factor);
            float z = glm::mix(_keys[index].value.z,
                _keys[next].value.z,
                factor);

            return glm::vec3(x, y, z);
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

