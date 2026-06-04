#pragma once
#ifndef DZEMIKK_CANVASSERIALIZER_H
#define DZEMIKK_CANVASSERIALIZER_H

#include "ecs/components/ui/canvas.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include <nlohmann/json.hpp>

namespace dzemikk {
    inline void to_json(nlohmann::json& j, const Canvas& canvas) {
        j["type"] = canvas.typeName();
        j["id"] = boost::uuids::to_string(canvas.getId());
    }
    inline void from_json(const nlohmann::json& json, Canvas& canvas) {
        if (!json.contains("type") || !json["type"].is_string() || json["type"] != canvas.typeName()) {
            throw std::runtime_error("Invalid component type for Canvas deserialization");
        }

        if (!json.contains("id")) {
            throw std::runtime_error("Missing fields for Canvas deserialization");
        }

        canvas.setId(boost::uuids::string_generator()(json["id"].get<std::string>()));

    }
    inline void registerCanvasSerializer(ComponentSerializerRegistry& registry) {
        registry.registerType(
            "Canvas",
            [](const Component& component) {
                const auto* canvas = dynamic_cast<const Canvas*>(&component);
                if (!canvas) throw std::runtime_error("Type mismatch in Canvas serialization");

                nlohmann::json j;
                to_json(j, *canvas);
                return j;
            },
            [](const ComponentSerializerRegistry::DeserializationContext context) {
                auto* canvas = context.gameObject.addComponent<Canvas>();
                from_json(context.json, *canvas);
            }
        );
    }
}
#endif
