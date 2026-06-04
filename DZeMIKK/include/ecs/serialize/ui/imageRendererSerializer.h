#pragma once
#ifndef DZEMIKK_IMAGERENDERERSERIALIZER_H
#define DZEMIKK_IMAGERENDERERSERIALIZER_H

#include "assetManager/assetmanager.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "renderer/material.h"

#include <nlohmann/json.hpp>

namespace dzemikk {
// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& j, const ImageRenderer& renderer) {
    j["type"] = renderer.typeName();
    j["id"] = boost::uuids::to_string(renderer.getId());
    j["color"] = {renderer.getColor()[0], renderer.getColor()[1], renderer.getColor()[2],
                  renderer.getColor()[3]};

    if (renderer.getMaterial()) {
        j["materialPath"] = renderer.getMaterial()->getShaderHandle().getAssetPath();
    }

    if (renderer.getTextureHandle().get() != nullptr) {
        j["texture"] = renderer.getTextureHandle().getAssetPath();
    } else {
        j["texture"] = "";
    }

}

inline void from_json(const nlohmann::json& json, ImageRenderer& renderer,
                      AssetManager* assetManager) {
    if (!json.contains("type") || !json["type"].is_string() ||
        json["type"] != renderer.typeName()) {
        throw std::runtime_error("Invalid component type for ImageRenderer deserialization");
    }
    if (!json.contains("id") || !json.contains("color") || !json["color"].is_array() ||
        json["color"].size() < 4 || !json.contains("materialPath") ||
        !json["materialPath"].is_string() || json["materialPath"].get<std::string>().empty()) {
        throw std::runtime_error("Missing fields for ImageRenderer deserialization");
    }
    renderer.setId(boost::uuids::string_generator()(json["id"].get<std::string>()));
    const auto& c = json["color"];
    renderer.setColor(
        glm::vec4(c[0].get<float>(), c[1].get<float>(), c[2].get<float>(), c[3].get<float>()));

    std::string texturePath = json.value("texture", "");
    if (!texturePath.empty()) {
        renderer.setTexture(assetManager->get<Texture>(texturePath));
    }

    renderer.setMesh(assetManager->getPrimitiveMesh(PrimitiveMeshLibrary::PrimitiveMesh::Quad));

    std::string shaderPath = json.value("materialPath", "");
    auto material = std::make_shared<dzemikk::Material>();
    material->setShader(assetManager->get<Shader>(shaderPath));
    renderer.setMaterial(material);
}

inline void registerImageRendererSerializer(ComponentSerializerRegistry& registry) {
    registry.registerType(
        "ImageRenderer",
        [](const Component& component) {
            const auto* renderer = dynamic_cast<const ImageRenderer*>(&component);
            if (!renderer) {
                throw std::runtime_error("Type mismatch in ImageRenderer serialization");
            }

            nlohmann::json j;
            to_json(j, *renderer);
            return j;
        },
        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto* renderer = context.gameObject.addComponent<ImageRenderer>();
            renderer->setRectTransform(context.gameObject.getComponent<RectTransform>());

            from_json(context.json, *renderer, context.assetManager);
        });
}
// NOLINTEND(readability-identifier-naming)
} // namespace dzemikk
#endif // DZEMIKK_IMAGERENDERERSERIALIZER_H
