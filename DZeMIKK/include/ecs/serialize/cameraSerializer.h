#pragma once
#ifndef DZEMIKK_CAMERASERIALIZER_H
#define DZEMIKK_CAMERASERIALIZER_H

#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/components/camera.h"
#include <nlohmann/json.hpp>

namespace dzemikk {
    inline void to_json(nlohmann::json& j, const Camera& camera) {
        j["type"] = camera.typeName();
        j["id"] = camera.getId();
        j["projectionType"] = camera.getProjectionType();
        j["near"] = camera.getNear();
        j["far"] = camera.getFar();
        j["fov"] = camera.getFov();
        j["aspect"] = camera.getAspect();
        j["left"] = camera.getLeft();
        j["right"] = camera.getRightOrtographic();
        j["bottom"] = camera.getBottom();
        j["top"] = camera.getTop();
    }

    inline void from_json(const nlohmann::json& json, Camera& camera) {
        if (!json.contains("type") || !json["type"].is_string() || json["type"] != camera.typeName()) {
            throw std::runtime_error("Invalid component type for Camera deserialization");
        }

        if (!json.contains("id") || !json.contains("projectionType")|| !json.contains("near")|| !json.contains("far")|| !json.contains("fov")|| !json.contains("aspect")|| !json.contains("left")|| !json.contains("right") || !json.contains("bottom") || !json.contains("top")) {
            throw std::runtime_error("Missing fields for Camera deserialization");
        }

        float nearPlane = json.at("near").get<float>();
        float farPlane = json.at("far").get<float>();
        auto type = static_cast<Camera::ProjectionType>(json.at("projectionType").get<int>());

        if (type == Camera::ProjectionType::Perspective) {
            camera.setPerspective(
                json.at("fov").get<float>(),
                json.at("aspect").get<float>(),
                nearPlane,
                farPlane
            );
        } else {
            camera.setOrthographic(
                json.at("left").get<float>(),
                json.at("right").get<float>(),
                json.at("bottom").get<float>(),
                json.at("top").get<float>(),
                nearPlane,
                farPlane
            );
        }
    }

    inline void registerCameraSerializer(ComponentSerializerRegistry& registry) {
        registry.registerType(
            "Camera",
            [](const Component& component) {
                const auto* camera = dynamic_cast<const Camera*>(&component);
                if (!camera) throw std::runtime_error("Type mismatch in Camera serialization");

                nlohmann::json j;
                to_json(j, *camera);
                return j;
            },
            [](ComponentSerializerRegistry::DeserializationContext context) {
                auto* camera = context.gameObject.addComponent<Camera>();
                from_json(context.json, *camera);
            }
        );
    }
}
#endif //TUL_PBL_DZEMIKK_CAMERASERIALIZER_H
