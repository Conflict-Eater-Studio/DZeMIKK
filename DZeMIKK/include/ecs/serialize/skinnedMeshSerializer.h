#pragma once
#ifndef DZEMIKK_SKINNEDMESHRENDERERSERIALIZER_H
#define DZEMIKK_SKINNEDMESHRENDERERSERIALIZER_H

#include "assetManager/assetmanager.h"
#include "ecs/components/skinnedMeshRenderer.h"
#include "ecs/serialize/componentSerializerRegistry.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

namespace dzemikk {

inline void to_json(nlohmann::json& json, const SkinnedMeshRenderer& renderer) {
    json["type"] = renderer.typeName();
    json["id"] = boost::uuids::to_string(renderer.getId());

    const auto& model = renderer.getModel();
    json["model"] = model.get() ? model.getAssetPath() : "";

    json["materials"] = nlohmann::json::array();

    for (const auto& material : renderer.getMaterials()) {
        if (!material) {
            json["materials"].push_back(nullptr);
            continue;
        }

        nlohmann::json m;

        m["shader"] =
            material->getShaderHandle().get() ? material->getShaderHandle().getAssetPath() : "";

        // TEXTURES
        m["albedoTexture"] = material->getAlbedoTextureHandle().get()
                                 ? material->getAlbedoTextureHandle().getAssetPath()
                                 : "";

        m["normalTexture"] = material->getNormalTextureHandle().get()
                                 ? material->getNormalTextureHandle().getAssetPath()
                                 : "";

        m["metallicTexture"] = material->getMetallicTextureHandle().get()
                                   ? material->getMetallicTextureHandle().getAssetPath()
                                   : "";

        m["roughnessTexture"] = material->getRoughnessTextureHandle().get()
                                    ? material->getRoughnessTextureHandle().getAssetPath()
                                    : "";

        m["aoTexture"] = material->getAOTextureHandle().get()
                             ? material->getAOTextureHandle().getAssetPath()
                             : "";

        m["emissiveTexture"] = material->getEmissiveTextureHandle().get()
                                   ? material->getEmissiveTextureHandle().getAssetPath()
                                   : "";

        // VALUES
        const auto c = material->getAlbedoColor();
        m["albedoColor"] = {c.x, c.y, c.z};

        m["metallicValue"] = material->getMetallic();
        m["roughnessValue"] = material->getRoughness();
        m["aoValue"] = material->getAO();

        const auto cE = material->getEmissiveColor();
        m["emissiveColor"] = {cE.x, cE.y, cE.z};
        m["emissiveValue"] = material->getEmissiveStrength();

        json["materials"].push_back(m);
    }
}

inline void from_json(const nlohmann::json& json, SkinnedMeshRenderer& renderer,
                      AssetManager* assetManager) {

    static boost::uuids::string_generator uuidGenerator;

    if (!json.contains("type") || json["type"] != renderer.typeName()) {
        throw std::runtime_error("Invalid type");
    }

    renderer.setId(uuidGenerator(json["id"].get<std::string>()));

    std::string modelPath = json.value("model", "");
    if (!modelPath.empty()) {
        if (modelPath.starts_with("primitive/")) {
            int idx = std::stoi(modelPath.substr(10));

            renderer.setModel(assetManager->getPrimitiveModel(
                static_cast<PrimitiveMeshLibrary::PrimitiveMesh>(idx)));
        } else {
            renderer.setModel(assetManager->get<Model>(modelPath));
        }
    }

    if (json.contains("materials")) {
        const auto& mats = json["materials"];

        for (size_t i = 0; i < mats.size(); i++) {
            if (!mats[i].is_object())
                continue;

            const auto& m = mats[i];

            auto material = std::make_shared<Material>();

            // shader
            if (m.contains("shader")) {
                auto shader = assetManager->get<Shader>(m["shader"].get<std::string>());
                material->setShader(shader);
            }

            // textures
            if (m.contains("albedoTexture") && m["albedoTexture"].is_string() &&
                !m["albedoTexture"].get<std::string>().empty()) {
                material->setAlbedoTexture(
                    assetManager->get<Texture>(m["albedoTexture"].get<std::string>()));
            }

            if (m.contains("normalTexture") && m["normalTexture"].is_string() &&
                !m["normalTexture"].get<std::string>().empty()) {
                material->setNormalTexture(
                    assetManager->get<Texture>(m["normalTexture"].get<std::string>()));
            }

            if (m.contains("metallicTexture") && m["metallicTexture"].is_string() &&
                !m["metallicTexture"].get<std::string>().empty()) {
                material->setMetallicTexture(
                    assetManager->get<Texture>(m["metallicTexture"].get<std::string>()));
            }

            if (m.contains("roughnessTexture") && m["roughnessTexture"].is_string() &&
                !m["roughnessTexture"].get<std::string>().empty()) {
                material->setRoughnessTexture(
                    assetManager->get<Texture>(m["roughnessTexture"].get<std::string>()));
            }

            if (m.contains("aoTexture") && m["aoTexture"].is_string() &&
                !m["aoTexture"].get<std::string>().empty()) {
                material->setAOTexture(
                    assetManager->get<Texture>(m["aoTexture"].get<std::string>()));
            }

            if (m.contains("emissiveTexture") && m["emissiveTexture"].is_string() &&
                !m["emissiveTexture"].get<std::string>().empty()) {
                material->setEmissiveTexture(
                    assetManager->get<Texture>(m["emissiveTexture"].get<std::string>()));
            }

            // values
            if (m.contains("albedoColor")) {
                auto c = m["albedoColor"];
                material->setAlbedoColor(glm::vec3(c[0], c[1], c[2]));
            }

            material->setMetallic(m.value("metallicValue", 0.0f));
            material->setRoughness(m.value("roughnessValue", 0.5f));
            material->setAO(m.value("aoValue", 1.0f));

            if (m.contains("emissiveColor")) {
                auto c = m["emissiveColor"];
                material->setEmissiveColor(glm::vec3(c[0], c[1], c[2]));
            }

            material->setEmissiveStrength(m.value("emissiveValue", 1.0f));

            renderer.setMaterial(i, material);
        }
    }

    renderer.setTransform(renderer.getOwner()->transform());
}

inline void registerSkinnedMeshRendererSerializer(ComponentSerializerRegistry& registry) {
    registry.registerType(
        "SkinnedMeshRenderer",
        [](const Component& component) {
            const auto* renderer = dynamic_cast<const SkinnedMeshRenderer*>(&component);
            if (!renderer) {
                throw std::runtime_error("Component type mismatch for SkinnedMeshRenderer");
            }

            nlohmann::json j;
            to_json(j, *renderer);
            return j;
        },
        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto* renderer = context.gameObject.addComponent<SkinnedMeshRenderer>();

            from_json(context.json, *renderer, context.assetManager);
        });
}

} // namespace dzemekk

#endif