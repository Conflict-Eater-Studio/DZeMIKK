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
        j["projectionType"] = static_cast<int>(camera.getProjectionType());
        j["near"] = camera.getNear();
        j["far"] = camera.getFar();
        j["fov"] = camera.getFov();
        j["aspect"] = camera.getAspect();
        j["left"] = camera.getLeft();
        j["right"] = camera.getRightOrtographic();
        j["bottom"] = camera.getBottom();
        j["top"] = camera.getTop();
    }

    inline void from_json(const nlohmann::json& j, Camera& camera) {
        float nearPlane = j.at("near").get<float>();
        float farPlane = j.at("far").get<float>();
        auto type = static_cast<Camera::ProjectionType>(j.at("projectionType").get<int>());

        if (type == Camera::ProjectionType::Perspective) {
            camera.setPerspective(
                j.at("fov").get<float>(),
                j.at("aspect").get<float>(),
                nearPlane,
                farPlane
            );
        } else {
            camera.setOrthographic(
                j.at("left").get<float>(),
                j.at("right").get<float>(),
                j.at("bottom").get<float>(),
                j.at("top").get<float>(),
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
