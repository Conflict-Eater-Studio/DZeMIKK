#pragma once
#ifndef DZEMIKK_TEXTRENDERERSERIALIZER_H
#define DZEMIKK_TEXTRENDERERSERIALIZER_H

#include <nlohmann/json.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>
#include "ecs/components/textRenderer.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "renderer/font.h"

namespace dzemikk {
    inline void to_json(nlohmann::json& json, const TextRenderer& textRenderer) {
        json["type"] = textRenderer.typeName();
        json["id"] = boost::uuids::to_string(textRenderer.getId());

        json["text"] = textRenderer.text;
        json["scale"] = textRenderer.scale;

        json["color"] = { textRenderer.color.r, textRenderer.color.g, textRenderer.color.b };

        if (textRenderer.font.get() != nullptr) {
            json["font"] = textRenderer.font.getAssetPath();
        } else {
            json["font"] = "";
        }
    }

    inline void from_json(const nlohmann::json& json, TextRenderer& textRenderer, AssetManager* assetManager) {
        static boost::uuids::string_generator uuidGenerator;

        if (!json.contains("type") || !json["type"].is_string() || json["type"] != textRenderer.typeName()) {
            throw std::runtime_error("Invalid component type for TextRenderer deserialization");
        }

        if (!json.contains("id") || !json.contains("text") || !json.contains("scale") || !json.contains("color") || json["color"].size() < 3 || !json["color"].is_array() || !json.contains("font")) {
            throw std::runtime_error("Missing fields for Transform deserialization");
        }

        textRenderer.setId(uuidGenerator(json["id"].get<std::string>()));
        textRenderer.text = json.value("text", "Hello");
        textRenderer.scale = json.value("scale", 1.0f);

        const auto& c = json["color"];
        textRenderer.color = glm::vec3(
            c[0].get<float>(),
            c[1].get<float>(),
            c[2].get<float>()
            );

        std::string fontPath = json.value("font", "");
        if (!fontPath.empty()) {
            textRenderer.font = assetManager->get<Font>(fontPath);
        }
    }

    inline void registerTextRendererSerializer(ComponentSerializerRegistry& registry) {
        registry.registerType(
            "TextRenderer",
            [](const Component& component) {
                const auto* renderer = dynamic_cast<const TextRenderer*>(&component);
                if (renderer == nullptr) {
                    throw std::runtime_error("Component type mismatch for TextRenderer serialization");
                }
                nlohmann::json j;
                to_json(j, *renderer);
                return j;
            },
            [](const ComponentSerializerRegistry::DeserializationContext& context) {
                auto* renderer = context.gameObject.addComponent<TextRenderer>();
                from_json(context.json, *renderer, context.assetManager);
            });
    }
}
#endif