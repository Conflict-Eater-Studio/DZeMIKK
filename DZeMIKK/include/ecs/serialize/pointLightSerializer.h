#pragma once
#ifndef DZEMIKK_POINT_LIGHT_SERIALIZER_H
#define DZEMIKK_POINT_LIGHT_SERIALIZER_H

#include "ecs/components/light/pointLight.h"
#include "ecs/serialize/componentSerializerRegistry.h"

#include <nlohmann/json.hpp>
#include <stdexcept>

namespace dzemikk {

inline void to_json(nlohmann::json& json, const PointLight& light) {

    json["type"] = "PointLight";

    json["color"] = {light.getColor().x, light.getColor().y, light.getColor().z};

    json["intensity"] = light.getIntensity();
    json["range"] = light.getRange();
    json["castsShadows"] = light.castsShadows();
}

inline void from_json(const nlohmann::json& json, PointLight& light) {

    if (!json.contains("type") || !json["type"].is_string() || json["type"] != "PointLight") {
        throw std::runtime_error("Invalid component type for PointLight deserialization");
    }

    if (!json.contains("color") || !json.contains("intensity") || !json.contains("range") ||
        !json.contains("castsShadows") ||

        !json["color"].is_array() || json["color"].size() != 3) {

        throw std::runtime_error("Missing or invalid fields for PointLight deserialization");
    }

    const auto& col = json["color"];

    light.setColor(glm::vec3(col[0].get<float>(), col[1].get<float>(), col[2].get<float>()));

    light.setIntensity(json["intensity"].get<float>());
    light.setRange(json["range"].get<float>());
    light.setCastsShadows(json["castsShadows"].get<bool>());
}

inline void registerPointLightSerializer(ComponentSerializerRegistry& registry) {

    registry.registerType(
        "PointLight",

        [](const Component& component) -> nlohmann::json {
            const auto* light = dynamic_cast<const PointLight*>(&component);

            if (!light) {
                throw std::runtime_error("Component type mismatch for PointLight serialization");
            }

            nlohmann::json json;
            to_json(json, *light);
            return json;
        },

        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto& gameObject = context.gameObject;

            auto* light = gameObject.addComponent<PointLight>();

            from_json(context.json, *light);
        });
}

} // namespace dzemikk

#endif // DZEMIKK_POINT_LIGHT_SERIALIZER_H