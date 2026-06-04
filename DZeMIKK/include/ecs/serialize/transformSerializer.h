#pragma once
#ifndef DZEMIKK_TRANSFORMSERIALIZER_H
#define DZEMIKK_TRANSFORMSERIALIZER_H

#include "ecs/components/transform.h"
#include "ecs/gameobject.h"
#include "ecs/serialize/componentSerializerRegistry.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>
#include <stdexcept>


namespace dzemikk {
    inline void to_json(nlohmann::json& json, const Transform& trs) {
        auto pos = trs.getPosition();
        auto rot = trs.getRotation();
        auto scale = trs.getScale();

        json["type"] = "Transform";
        json["id"] = boost::uuids::to_string(trs.getId());
        json["position"] = {pos[0], pos[1], pos[2]};
        json["rotation"] = {rot[3], rot[0], rot[1], rot[2]};
        json["scale"] = {scale[0], scale[1], scale[2]};
    }

    inline void from_json(const nlohmann::json& json, Transform& trs) {
        static boost::uuids::string_generator uuidGenerator;

        if (!json.contains("type") || !json["type"].is_string() || json["type"] != "Transform") {
            throw std::runtime_error("Invalid component type for Transform deserialization");
        }

        if (!json.contains("id") || !json.contains("position") || !json.contains("rotation") ||
            !json.contains("scale") || !json["position"].is_array() || json["position"].size() != 3 ||
            !json["rotation"].is_array() || json["rotation"].size() != 4 || !json["scale"].is_array() ||
            json["scale"].size() != 3) {
            throw std::runtime_error("Missing fields for Transform deserialization");
        }

        const auto& pos = json["position"];
        const auto& rot = json["rotation"];
        const auto& scale = json["scale"];

        trs.setId(uuidGenerator(json["id"].get<std::string>()));
        trs.setPosition(glm::vec3(pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>()));
        trs.setRotation(glm::quat(rot[0].get<float>(), rot[1].get<float>(), rot[2].get<float>(),
                                  rot[3].get<float>()));
        trs.setScale(glm::vec3(scale[0].get<float>(), scale[1].get<float>(), scale[2].get<float>()));
    }

    inline void registerTransformSerializer(ComponentSerializerRegistry& registry) {
        registry.registerType(
            "Transform",
            [](const Component& component) {
                const auto* transform = dynamic_cast<const Transform*>(&component);
                if (transform == nullptr) {
                    throw std::runtime_error("Component type mismatch for Transform serialization");
                }
                return nlohmann::json(*transform);
            },
            [](const ComponentSerializerRegistry::DeserializationContext& context) {
                auto& gameObject = context.gameObject;
                from_json(context.json, *gameObject.transform());
            });
    }
}

#endif