#pragma once
#ifndef DZEMIKK_PROPERTYKEY_H
#define DZEMIKK_PROPERTYKEY_H
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>      // optional but safe
#include <glm/gtc/quaternion.hpp>    // keep
#include <glm/gtx/quaternion.hpp>

namespace glm {
    inline void to_json(nlohmann::json& j, const glm::vec3& v) {
        j = { v.x, v.y, v.z };
    }

    inline void from_json(const nlohmann::json& j, glm::vec3& v) {
        v.x = j.at(0);
        v.y = j.at(1);
        v.z = j.at(2);
    }
    inline void to_json(nlohmann::json& j, const glm::quat& q) {
        j = { q.x, q.y, q.z, q.w };
    }

    inline void from_json(const nlohmann::json& j, glm::quat& q) {
        q.x = j.at(0);
        q.y = j.at(1);
        q.z = j.at(2);
        q.w = j.at(3);
    }
}

template<typename T>
struct PropertyKey {
    float time;
    T value;
};


template<typename T>
void to_json(nlohmann::json& json, const PropertyKey<T>& key) {
    json["time"] = key.time;
    json["value"] = key.value;
}

template<typename T>
void from_json(const nlohmann::json& json, PropertyKey<T>& key) {
    key.time = json["time"];
    key.value = json["value"];
}

#endif