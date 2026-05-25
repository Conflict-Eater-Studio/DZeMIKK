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

        if (spriteRenderer.getMaterial() != nullptr) {

            auto shaderHandle = spriteRenderer.getMaterial()->getShaderHandle();

            if (shaderHandle.get() != nullptr) {
                json["shader"] = shaderHandle.getAssetPath();
            } else {
                json["shader"] = "";
            }

        } else {
            json["shader"] = "";
        }
    }

    inline void from_json(const nlohmann::json& json, SpriteRenderer& spriteRenderer, AssetManager* assetManager) {
        static boost::uuids::string_generator uuidGenerator;

        if (!json.contains("type") || !json["type"].is_string() || json["type"] != spriteRenderer.typeName()) {
            throw std::runtime_error("Invalid component type for SpriteRenderer deserialization");
        }

        if (!json.contains("id") || !json.contains("color") || json["color"].size() < 4 || !json["color"].is_array() || !json.contains("texture")) {
            throw std::runtime_error("Missing fields for Transform deserialization");
        }

        spriteRenderer.setId(uuidGenerator(json["id"].get<std::string>()));

        const auto& c = json["color"];
        spriteRenderer.setColor(glm::vec4(
            c[0].get<float>(),
            c[1].get<float>(),
            c[2].get<float>(),
            c[3].get<float>()
        ));

        std::string texturePath = json.value("texture", "");
        if (!texturePath.empty()) {
            spriteRenderer.setTexture(assetManager->get<Texture>(texturePath));
        }

        spriteRenderer.setMesh(
            assetManager->getPrimitive(PrimitiveMeshLibrary::PrimitiveMesh::Quad));

            std::string shaderPath = json.value("shader", "");

        if (!shaderPath.empty()) {

            auto* material = new Material();

            material->setShader(assetManager->get<Shader>(shaderPath));

            spriteRenderer.setMaterial(material);
        }

        spriteRenderer.setTransform(spriteRenderer.getOwner()->transform());
    }

    inline void registerSpriteRendererSerializer(ComponentSerializerRegistry& registry) {
        registry.registerType(
            "SpriteRenderer",
            [](const Component& component) {
                const auto* renderer = dynamic_cast<const SpriteRenderer*>(&component);
                if (renderer == nullptr) {
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