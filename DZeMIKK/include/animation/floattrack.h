#pragma once
#ifndef DZEMIKK_FLOATTRACK_H
#define DZEMIKK_FLOATTRACK_H

#include "glm/ext/quaternion_common.hpp"
#include "property.h"
#include "propertykey.h"


namespace dzemikk {
using FloatPropertyKey = PropertyKey<float>;
using FloatProperty = Property<float>;

class FloatTrack : public IAnimationTrack {
    public:
        void apply(float time) override {
            if (_keys.empty()) {
                return;
            }
            float value = sample(time);
            _property.set(value);
        }
        void addKey(FloatPropertyKey key) {
                _keys.push_back(key);
        }
        void setKeys(std::vector<FloatPropertyKey> keys) {
            _keys = keys;
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

      public:
        std::string getType() const override {
            return "FloatTrack";
        }
        nlohmann::json serialize() const override {
            nlohmann::json j;
            j["keys"] = _keys;
            return j;
        }
};
}
#endif
