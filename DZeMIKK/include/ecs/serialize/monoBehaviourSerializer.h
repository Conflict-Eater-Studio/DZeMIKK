#ifndef DZEMIKK_MONOBEHAVIOURSERIALIZER_H
#define DZEMIKK_MONOBEHAVIOURSERIALIZER_H

#pragma once

#include "ecs/components/monoBehaviour.h"
#include "ecs/serialize/serializedRef.h"
#include "ecs/serialize/uuid.h"

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace dzemikk {
class MonoBehaviourSerializer {
  public:
    static void writeBase(nlohmann::json& json, const MonoBehaviour& mono,
                          const std::string& typeName) {
        json["type"] = typeName;
        json["id"] = mono.getId();
    }

    static void readBase(const nlohmann::json& json, MonoBehaviour& mono,
                         const std::string& expectedTypeName) {
        if (!json.contains("type") || !json["type"].is_string() ||
            json["type"].get<std::string>() != expectedTypeName) {
            throw std::runtime_error("Invalid component type for " + expectedTypeName);
        }

        if (!json.contains("id")) {
            throw std::runtime_error("Missing field 'id' for " + expectedTypeName);
        }

        mono.setId(json["id"].get<boost::uuids::uuid>());
    }

    template <typename T>
    static void writeRef(nlohmann::json& json, const char* fieldName, const SerializedRef<T>& ref) {
        const boost::uuids::uuid refId = ref.toSerializedId();
        if (refId.is_nil()) {
            json[fieldName] = nullptr;
            return;
        }

        json[fieldName] = refId;
    }

    template <typename T>
    static void readRef(const nlohmann::json& json, const char* fieldName, SerializedRef<T>& ref) {
        if (!json.contains(fieldName) || json[fieldName].is_null()) {
            ref.clear();
            return;
        }

        ref.captureSerializedId(json[fieldName].get<boost::uuids::uuid>());
    }
};
} // namespace dzemikk

#endif // DZEMIKK_MONOBEHAVIOURSERIALIZER_H
