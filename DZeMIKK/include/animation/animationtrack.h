#ifndef DZEMIKK_ANIMATIONTRACK_H
#define DZEMIKK_ANIMATIONTRACK_H
#pragma once

#include "ecs/components/transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace dzemikk {

// class FloatProperty {
// public:
//     FloatProperty(float* ptr) : _ptr(ptr) {}
//
//     void set(float value) {
//         *_ptr = value;
//     }
//
//     float get() const {
//         return *_ptr;
//     }
//
// private:
//     float* _ptr;
// };

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

class AnimationTrack {
public:
    AnimationTrack();
    AnimationTrack(std::string name);

    const std::string& getName() const noexcept;

    void interpolate(float time);

    void setProperty(const FloatProperty& property);
    FloatProperty getProperty() const;

    const std::vector<FloatPropertyKey>& getKeys() const noexcept;
    void addKey(FloatPropertyKey key);
    void setKeys(std::vector<FloatPropertyKey> keys);

private:
    std::string _name;
    FloatProperty _property;
    std::vector<FloatPropertyKey> _keys;

    size_t findPropertyIndex(float time) const;
};

}
#endif
