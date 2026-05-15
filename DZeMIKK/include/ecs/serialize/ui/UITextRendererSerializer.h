#pragma once
#ifndef DZEMIKK_UI_TEXT_RENDERER_SERIALIZER_H
#define DZEMIKK_UI_TEXT_RENDERER_SERIALIZER_H

#include "assetManager/assetmanager.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "renderer/font.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

namespace dzemikk {

inline void to_json(nlohmann::json& json, const UITextRenderer& textRenderer) {

    json["type"] = textRenderer.typeName();

    json["id"] = boost::uuids::to_string(textRenderer.getId());

    json["text"] = textRenderer.text;

    json["scale"] = textRenderer.scale;

    json["color"] = {textRenderer.color.r, textRenderer.color.g, textRenderer.color.b};

    json["horizontalAlign"] = static_cast<uint8_t>(textRenderer.horizontalAlign);

    json["verticalAlign"] = static_cast<uint8_t>(textRenderer.verticalAlign);

    if (textRenderer.fontAsset.get() != nullptr) {
        json["font"] = textRenderer.fontAsset.getAssetPath();
    } else {
        json["font"] = "";
    }
}

inline void from_json(const nlohmann::json& json, UITextRenderer& textRenderer,
                      AssetManager* assetManager) {

    static boost::uuids::string_generator uuidGenerator;

    if (!json.contains("type") || !json["type"].is_string() ||
        json["type"] != textRenderer.typeName()) {

        throw std::runtime_error("Invalid component type for UITextRenderer deserialization");
    }

    if (!json.contains("id") || !json.contains("text") || !json.contains("scale") ||
        !json.contains("color") || !json["color"].is_array() || json["color"].size() < 3 ||
        !json.contains("horizontalAlign") || !json.contains("verticalAlign") ||
        !json.contains("font")) {

        throw std::runtime_error("Missing fields for UITextRenderer deserialization");
    }

    textRenderer.setId(uuidGenerator(json["id"].get<std::string>()));

    textRenderer.text = json["text"].get<std::string>();

    textRenderer.scale = json["scale"].get<float>();

    const auto& c = json["color"];

    textRenderer.color = glm::vec3(c[0].get<float>(), c[1].get<float>(), c[2].get<float>());

    textRenderer.horizontalAlign =
        static_cast<UITextRenderer::HorizontalAlign>(json["horizontalAlign"].get<uint8_t>());

    textRenderer.verticalAlign =
        static_cast<UITextRenderer::VerticalAlign>(json["verticalAlign"].get<uint8_t>());

    std::string fontPath = json.value("font", "");

    if (!fontPath.empty()) {

        textRenderer.fontAsset = assetManager->get<Font>(fontPath);

        textRenderer.font = textRenderer.fontAsset.get();
    }
}

inline void registerUITextRendererSerializer(ComponentSerializerRegistry& registry) {

    registry.registerType(
        "UITextRenderer",

        [](const Component& component) {
            const auto* textRenderer = dynamic_cast<const UITextRenderer*>(&component);

            if (textRenderer == nullptr) {

                throw std::runtime_error(
                    "Component type mismatch for UITextRenderer serialization");
            }

            nlohmann::json json;

            to_json(json, *textRenderer);

            return json;
        },

        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto* textRenderer = context.gameObject.addComponent<UITextRenderer>();

            from_json(context.json, *textRenderer, context.assetManager);
        });
}

} // namespace dzemikk

#endif // DZEMIKK_UI_TEXT_RENDERER_SERIALIZER_H