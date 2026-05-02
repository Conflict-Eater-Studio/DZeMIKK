#pragma once
#ifndef DZEMIKK_VECTORTRACK_H
#define DZEMIKK_VECTORTRACK_H

#include "IAnimationTrack.h"
#include <glm/glm.hpp>
#include "ecs/components/transform.h"

namespace dzemikk {

using VectorPropertyKey = PropertyKey<glm::vec3>;
using VectorProperty = Property<glm::vec3>;

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
        void bindPosition(Transform& transform) {
            _property = VectorProperty(
                [&transform]() { return transform.getPosition(); },
                [&transform](const glm::vec3& value) { transform.setPosition(value); }
            );
        }
        void bindScale(Transform& transform) {
            _property = VectorProperty(
                [&transform]() { return transform.getScale(); },
                [&transform](const glm::vec3& value) { transform.setScale(value); }
            );
        }
        [[nodiscard]] VectorProperty getProperty() const {
            return _property;
        }

        [[nodiscard]] glm::vec3 sample(float time) const {
            if (_keys.size() == 1) {
                return _keys[0].value;
            }

            size_t index = findPropertyIndex(time);
            size_t next = index + 1;

            float t1 = _keys[index].time;
            float t2 = _keys[next].time;

            float factor = (time - t1) / (t2 - t1);

            float x = glm::mix(_keys[index].value.x, _keys[next].value.x, factor);
            float y = glm::mix(_keys[index].value.y, _keys[next].value.y, factor);
            float z = glm::mix(_keys[index].value.z, _keys[next].value.z, factor);

            return {x, y, z};
        }
    private:
        VectorProperty _property;
        std::vector<VectorPropertyKey> _keys;

        [[nodiscard]] size_t findPropertyIndex(float time) const {
            for (size_t i = 0; i < _keys.size() - 1; i++) {
                if (time < _keys[i + 1].time)
                    return i;
            }
            return _keys.size() - 2;
        }

      public:
        std::string getType() const override {
            return "VectorTrack";
        }
        nlohmann::json serialize() const override {
            nlohmann::json j;
            j["keys"] = _keys;
            return j;
        }
};
}
#endif

