#pragma once
#ifndef DZEMIKK_SPRITERENDERERSERIALIZER_H
#define DZEMIKK_SPRITERENDERERSERIALIZER_H

#include "ecs/components/spriteRenderer.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "assetManager/assetmanager.h"
#include "renderer/texture.h"

#include <nlohmann/json.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

namespace dzemikk {

    inline void to_json(nlohmann::json& json, const SpriteRenderer& spriteRenderer) {
        json["type"] = spriteRenderer.typeName();
        json["id"] = boost::uuids::to_string(spriteRenderer.getId());

        const auto& color = spriteRenderer.getColor();
        json["color"] = { color.r, color.g, color.b, color.a };

        if (spriteRenderer.getTextureHandle().get() != nullptr) {
            json["texture"] = spriteRenderer.getTextureHandle().getAssetPath();
        } else {
            json["texture"] = "";
        }
    }

    inline void from_json(const nlohmann::json& json, SpriteRenderer& spriteRenderer, AssetManager* assetManager) {
        static boost::uuids::string_generator uuidGenerator;

        if (json.contains("id") && json["id"].is_string()) {
            try {
                spriteRenderer.setId(uuidGenerator(json["id"].get<std::string>()));
            } catch (const std::exception& e) {
    #if DZEMIKK_DEV_TOOLS
                spdlog::warn("Malformed UUID in SpriteRenderer JSON: {}", e.what());
    #endif
            }
        }

        if (json.contains("color") && json["color"].is_array() && json["color"].size() >= 4) {
            const auto& c = json["color"];
            spriteRenderer.setColor(glm::vec4(
                c[0].get<float>(),
                c[1].get<float>(),
                c[2].get<float>(),
                c[3].get<float>()
            ));
        }

        std::string texturePath = json.value("texture", "");
        if (!texturePath.empty()) {
            spriteRenderer.setTexture(assetManager->get<Texture>(texturePath));
        }
    }

    inline void registerSpriteRendererSerializer(ComponentSerializerRegistry& registry) {
        registry.registerType(
            "SpriteRenderer",
            [](const Component& component) {
                const auto* renderer = dynamic_cast<const SpriteRenderer*>(&component);
                if (renderer == nullptr) {
    #if DZEMIKK_DEV_TOOLS
                    spdlog::error("Component type mismatch during SpriteRenderer serialization!");
    #endif
                    throw std::runtime_error("Component type mismatch for SpriteRenderer serialization");
                }
                nlohmann::json j;
                to_json(j, *renderer);
                return j;
            },
            [](const ComponentSerializerRegistry::DeserializationContext& context) {
                auto* renderer = context.gameObject.addComponent<SpriteRenderer>();
                from_json(context.json, *renderer, context.assetManager);
            });
    }
}
#endif