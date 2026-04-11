#ifndef DZEMIKK_FLOATTRACK_H
#define DZEMIKK_FLOATTRACK_H
#include "IAnimationTrack.h"
#include "glm/ext/quaternion_common.hpp"

#include <functional>

namespace dzemikk {

class FloatProperty {
public:
    using Getter = std::function<float()>;
    using Setter = std::function<void(float)>;

    FloatProperty() = default;
    FloatProperty(Getter g, Setter s)
        : _get(std::move(g)), _set(std::move(s)) {}

    void set(float value) {
        _set(value);
    }

    float get() const {
        return _get();
    }

private:
    Getter _get;
    Setter _set;
};

struct FloatPropertyKey {
    float time;
    float value;
};

class FloatTrack : public IAnimationTrack {
    public:
        void apply(float time) override {
            if (!_property.get() || _keys.empty()) return;

            float value = sample(time);
            _property.set(value);
        }
        void addKey(FloatPropertyKey key) {
                _keys.push_back(key);
        }
        void setProperty(const FloatProperty& property) {
            _property = property;
        }
        FloatProperty getProperty() const {
            return _property;
        }
    private:
        FloatProperty _property;
        std::vector<FloatPropertyKey> _keys;
        float sample(float time) const {
            if (_keys.size() == 1) {
                return _keys[0].value;
            }

            size_t index = findPropertyIndex(time);
            size_t next = index + 1;

            float t1 = _keys[index].time;
            float t2 = _keys[next].time;

            float factor = (time - t1) / (t2 - t1);

            float value = glm::mix(_keys[index].value,
                            _keys[next].value,
                            factor);
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
