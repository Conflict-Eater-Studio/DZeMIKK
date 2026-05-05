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

inline void from_json(const nlohmann::json& j, ImageRenderer& renderer, AssetManager* assetManager) {
    if (j.contains("id")) {
        renderer.setId(boost::uuids::string_generator()(j["id"].get<std::string>()));
    }

    if (j.contains("color") && j["color"].is_array()) {
        renderer.setColor(glm::vec4(j["color"][0], j["color"][1], j["color"][2], j["color"][3]));
    }

    if (j.contains("meshPath") && !j["meshPath"].is_null()) {
        renderer.setMesh(assetManager->get<Mesh>(j["meshPath"]));
    }

    if (j.contains("materialPath") && !j["materialPath"].is_null()) {
        Material* material = nullptr;
        material->setShader(assetManager->get<Shader>(j["materialPath"]));
        renderer.setMaterial(material);
    }
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
