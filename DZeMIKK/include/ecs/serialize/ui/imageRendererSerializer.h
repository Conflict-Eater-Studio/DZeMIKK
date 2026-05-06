#pragma once
#ifndef DZEMIKK_IMAGERENDERERSERIALIZER_H
#define DZEMIKK_IMAGERENDERERSERIALIZER_H

#include "assetManager/assetmanager.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "renderer/material.h"

#include <nlohmann/json.hpp>

namespace dzemikk {
inline void to_json(nlohmann::json& j, const ImageRenderer& renderer) {
    j["type"] = renderer.typeName();
    j["id"] = boost::uuids::to_string(renderer.getId());
    j["color"] = {renderer.getColor().r, renderer.getColor().g, renderer.getColor().b, renderer.getColor().a};

    if (renderer.getMesh()) {
        j["meshPath"] = renderer.getMeshHandle().getAssetPath();
    }
    if (renderer.getMaterial()) {
        j["materialPath"] = renderer.getMaterial()->getShaderHandle().getAssetPath();
    }
}

inline void from_json(const nlohmann::json& json, ImageRenderer& renderer, AssetManager* assetManager) {
    if (!json.contains("type") || !json["type"].is_string() || json["type"] != renderer.typeName()) {
        throw std::runtime_error("Invalid component type for ImageRenderer deserialization");
    }
    if (!json.contains("id") || !json.contains("color") || !json["color"].is_array() || !json.contains("meshPath")  || !json.contains("materialPath")) {
        throw std::runtime_error("Missing fields for UICheckbox deserialization");
        }
    renderer.setId(boost::uuids::string_generator()(json["id"].get<std::string>()));
    renderer.setColor(glm::vec4(json["color"][0], json["color"][1], json["color"][2], json["color"][3]));

    std::string meshPath = json.value("meshPath", "");
    renderer.setMesh(assetManager->get<Mesh>(meshPath));

    std::string shaderPath = json.value("materialPath", "");
    std::shared_ptr<dzemikk::Material> material = nullptr;
    material->setShader(assetManager->get<Shader>(shaderPath));
    renderer.setMaterial(material);
}

inline void registerImageRendererSerializer(ComponentSerializerRegistry& registry) {
    registry.registerType(
        "ImageRenderer",
        [](const Component& component) {
            const auto* renderer = dynamic_cast<const ImageRenderer*>(&component);
            if (!renderer) throw std::runtime_error("Type mismatch in ImageRenderer serialization");

            nlohmann::json j;
            to_json(j, *renderer);
            return j;
        },
        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto* renderer = context.gameObject.addComponent<ImageRenderer>();
            renderer->setRectTransform(context.gameObject.getComponent<RectTransform>());

            from_json(context.json, *renderer, context.assetManager);
        }
    );
}
}
#endif //DZEMIKK_IMAGERENDERERSERIALIZER_H
