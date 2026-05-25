#pragma once
#ifndef DZEMIKK_DIRECTIONAL_LIGHT_SERIALIZER_H
#define DZEMIKK_DIRECTIONAL_LIGHT_SERIALIZER_H

#include "ecs/components/light/directionalLight.h"
#include "ecs/serialize/componentSerializerRegistry.h"

#include <nlohmann/json.hpp>
#include <stdexcept>

namespace dzemikk {

inline void to_json(nlohmann::json& json, const DirectionalLight& light) {

    json["type"] = "DirectionalLight";

    json["direction"] = {light.getDirection().x, light.getDirection().y, light.getDirection().z};

    json["color"] = {light.getColor().x, light.getColor().y, light.getColor().z};

    json["intensity"] = light.getIntensity();
    json["castsShadows"] = light.castsShadows();
}

inline void from_json(const nlohmann::json& json, DirectionalLight& light) {

    if (!json.contains("type") || !json["type"].is_string() || json["type"] != "DirectionalLight") {
        throw std::runtime_error("Invalid component type for DirectionalLight deserialization");
    }

    if (!json.contains("direction") || !json.contains("color") || !json.contains("intensity") ||
        !json.contains("castsShadows") ||

        !json["direction"].is_array() || json["direction"].size() != 3 ||
        !json["color"].is_array() || json["color"].size() != 3) {

        throw std::runtime_error("Missing or invalid fields for DirectionalLight deserialization");
    }

    const auto& dir = json["direction"];
    const auto& col = json["color"];

    light.setDirection(glm::vec3(dir[0].get<float>(), dir[1].get<float>(), dir[2].get<float>()));

    light.setColor(glm::vec3(col[0].get<float>(), col[1].get<float>(), col[2].get<float>()));

    light.setIntensity(json["intensity"].get<float>());
    light.setCastsShadows(json["castsShadows"].get<bool>());
}

inline void registerDirectionalLightSerializer(ComponentSerializerRegistry& registry) {

    registry.registerType(
        "DirectionalLight",

        [](const Component& component) -> nlohmann::json {
            const auto* light = dynamic_cast<const DirectionalLight*>(&component);

            if (!light) {
                throw std::runtime_error(
                    "Component type mismatch for DirectionalLight serialization");
            }

            nlohmann::json json;
            to_json(json, *light);
            return json;
        },

        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto& gameObject = context.gameObject;

            auto* light = gameObject.addComponent<DirectionalLight>();

            from_json(context.json, *light);
        });
}

} // namespace dzemikk

#endif // DZEMIKK_DIRECTIONAL_LIGHT_SERIALIZER_H