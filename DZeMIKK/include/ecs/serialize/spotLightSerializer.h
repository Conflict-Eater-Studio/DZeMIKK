#pragma once
#ifndef DZEMIKK_SPOT_LIGHT_SERIALIZER_H
#define DZEMIKK_SPOT_LIGHT_SERIALIZER_H

#include "ecs/components/light/spotLight.h"
#include "ecs/serialize/componentSerializerRegistry.h"

#include <nlohmann/json.hpp>
#include <stdexcept>

namespace dzemikk {

inline void to_json(nlohmann::json& json, const SpotLight& light) {

    json["type"] = "SpotLight";

    json["direction"] = {light.getDirection().x, light.getDirection().y, light.getDirection().z};

    json["color"] = {light.getColor().x, light.getColor().y, light.getColor().z};

    json["intensity"] = light.getIntensity();
    json["range"] = light.getRange();
    json["innerCutoff"] = light.getInnerCutoff();
    json["outerCutoff"] = light.getOuterCutoff();
    json["castsShadows"] = light.castsShadows();
}

inline void from_json(const nlohmann::json& json, SpotLight& light) {

    if (!json.contains("type") || !json["type"].is_string() || json["type"] != "SpotLight") {
        throw std::runtime_error("Invalid component type for SpotLight deserialization");
    }

    if (!json.contains("direction") || !json.contains("color") || !json.contains("intensity") ||
        !json.contains("range") || !json.contains("innerCutoff") || !json.contains("outerCutoff") ||
        !json.contains("castsShadows") ||

        !json["direction"].is_array() || json["direction"].size() != 3 ||

        !json["color"].is_array() || json["color"].size() != 3) {

        throw std::runtime_error("Missing or invalid fields for SpotLight deserialization");
    }

    const auto& dir = json["direction"];
    const auto& col = json["color"];

    light.setDirection(glm::vec3(dir[0].get<float>(), dir[1].get<float>(), dir[2].get<float>()));

    light.setColor(glm::vec3(col[0].get<float>(), col[1].get<float>(), col[2].get<float>()));

    light.setIntensity(json["intensity"].get<float>());
    light.setRange(json["range"].get<float>());

    light.setInnerCutoff(json["innerCutoff"].get<float>());
    light.setOuterCutoff(json["outerCutoff"].get<float>());

    light.setCastsShadows(json["castsShadows"].get<bool>());
}

inline void registerSpotLightSerializer(ComponentSerializerRegistry& registry) {

    registry.registerType(
        "SpotLight",

        [](const Component& component) -> nlohmann::json {
            const auto* light = dynamic_cast<const SpotLight*>(&component);

            if (!light) {
                throw std::runtime_error("Component type mismatch for SpotLight serialization");
            }

            nlohmann::json json;
            to_json(json, *light);
            return json;
        },

        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto& gameObject = context.gameObject;

            auto* light = gameObject.addComponent<SpotLight>();

            from_json(context.json, *light);
        });
}

} // namespace dzemikk

#endif // DZEMIKK_SPOT_LIGHT_SERIALIZER_H