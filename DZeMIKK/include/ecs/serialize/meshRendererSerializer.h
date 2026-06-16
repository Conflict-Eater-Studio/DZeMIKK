#pragma once
#ifndef DZEMIKK_MESHRENDERERSERIALIZER_H
#define DZEMIKK_MESHRENDERERSERIALIZER_H

#include "ecs/components/meshRenderer.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/serialize/uuid.h"
#include "renderer/material.h"
#include "renderer/model.h"
#include "renderer/shader.h"

#include <nlohmann/json.hpp>

namespace dzemikk {
void to_json(nlohmann::json& json, const MeshRenderer& meshRenderer) {
    json["type"] = meshRenderer.typeName();

    json["id"] = boost::uuids::to_string(meshRenderer.getId());

    const auto& modelHandle = meshRenderer.getModelHandle();
    json["model"] = (modelHandle.get() != nullptr) ? modelHandle.getAssetPath() : "";

    json["materials"] = nlohmann::json::array();
    const auto& materials = meshRenderer.getMaterials();

    for (const std::shared_ptr<Material> material : materials) {
        if (!material) {
            json["materials"].push_back(nullptr);
            continue;
        }

        nlohmann::json matJson;

        const auto& shader = material->getShaderHandle();
        matJson["shader"] = shader.get() ? shader.getAssetPath() : "";

        matJson["textures"]["albedo"] = material->getAlbedoTextureHandle().get()
                                            ? material->getAlbedoTextureHandle().getAssetPath()
                                            : "";

        matJson["textures"]["normal"] = material->getNormalTextureHandle().get()
                                            ? material->getNormalTextureHandle().getAssetPath()
                                            : "";

        matJson["textures"]["metallic"] = material->getMetallicTextureHandle().get()
                                              ? material->getMetallicTextureHandle().getAssetPath()
                                              : "";

        matJson["textures"]["roughness"] =
            material->getRoughnessTextureHandle().get()
                ? material->getRoughnessTextureHandle().getAssetPath()
                : "";

        matJson["textures"]["ao"] = material->getAOTextureHandle().get()
                                        ? material->getAOTextureHandle().getAssetPath()
                                        : "";

        matJson["textures"]["emissive"] = material->getEmissiveTextureHandle().get()
                                              ? material->getEmissiveTextureHandle().getAssetPath()
                                              : "";

        matJson["pbr"]["albedo"] = {material->getAlbedoColor().r, material->getAlbedoColor().g,
                                    material->getAlbedoColor().b};

        matJson["pbr"]["metallic"] = material->getMetallic();
        matJson["pbr"]["roughness"] = material->getRoughness();
        matJson["pbr"]["ao"] = material->getAO();

        matJson["pbr"]["emissive"] = {material->getEmissiveColor().r,
                                      material->getEmissiveColor().g,
                                      material->getEmissiveColor().b};
        matJson["pbr"]["emissiveStrength"] = material->getEmissiveStrength();

        json["materials"].push_back(matJson);
    }
}

inline void from_json(const nlohmann::json& json, MeshRenderer& meshRenderer,
                      AssetManager* assetManager) {
    static boost::uuids::string_generator uuidGenerator;

    if (!json.contains("type") || !json["type"].is_string() ||
        json["type"] != meshRenderer.typeName()) {
        throw std::runtime_error("Invalid component type for MeshRenderer deserialization");
    }

    if (!json.contains("id") || !json.contains("model") ||
        !json.contains("materials") || !json["materials"].is_array()) {
        throw std::runtime_error("Missing fields for MeshRenderer deserialization");
    }

    meshRenderer.setId(uuidGenerator(json["id"].get<std::string>()));

    std::string modelPath = json.value("model", "");

    if (!modelPath.empty()) {

        constexpr std::string_view primitivePrefix = "primitive/";

        if (modelPath.starts_with(primitivePrefix)) {

            std::string primitiveIndexString = modelPath.substr(primitivePrefix.size());

            int primitiveIndex = std::stoi(primitiveIndexString);

            meshRenderer.setModel(assetManager->getPrimitiveModel(
                static_cast<PrimitiveMeshLibrary::PrimitiveMesh>(primitiveIndex)));

        }

        else {

            meshRenderer.setModel(assetManager->get<Model>(modelPath));
        }
    }

    if (json.contains("materials") && json["materials"].is_array()) {

        const auto& arr = json["materials"];

        for (size_t i = 0; i < arr.size(); i++) {

            if (!arr[i].is_object()) {
                continue;
            }

            const auto& matJson = arr[i];

            auto material = std::make_shared<Material>();

            std::string shaderPath = matJson.value("shader", "");
            if (!shaderPath.empty()) {
                material->setShader(assetManager->get<Shader>(shaderPath));
            }

            auto loadTex = [&](const char* key) -> AssetHandle<Texture> {
                std::string path = matJson["textures"].value(key, "");
                if (!path.empty()) {
                    return assetManager->get<Texture>(path);
                }
                return {};
            };

            material->setAlbedoTexture(loadTex("albedo"));
            material->setNormalTexture(loadTex("normal"));
            material->setMetallicTexture(loadTex("metallic"));
            material->setRoughnessTexture(loadTex("roughness"));
            material->setAOTexture(loadTex("ao"));
            material->setEmissiveTexture(loadTex("emissive"));

            if (matJson.contains("pbr")) {
                const auto& pbr = matJson["pbr"];

                auto col = pbr.value("albedo", std::vector<float>{1, 1, 1});
                if (col.size() == 3) {
                    material->setAlbedoColor(glm::vec3(col[0], col[1], col[2]));
                }

                material->setMetallic(pbr.value("metallic", 0.0f));
                material->setRoughness(pbr.value("roughness", 0.5f));
                material->setAO(pbr.value("ao", 1.0f));

                col = pbr.value("emissive", std::vector<float>{1, 1, 1});
                if (col.size() == 3) {
                    material->setEmissiveColor(glm::vec3(col[0], col[1], col[2]));
                }

                material->setEmissiveStrength(pbr.value("emissiveStrength", 1.0f));
            }

            meshRenderer.setMaterial(i, material);
        }
    }

    meshRenderer.setTransform(meshRenderer.getOwner()->transform());
}

inline void registerMeshRendererSerializer(ComponentSerializerRegistry& registry) {
    registry.registerType(
        "MeshRenderer",
        [](const Component& component) {
            const auto* meshRenderer = dynamic_cast<const MeshRenderer*>(&component);
            if (meshRenderer == nullptr) {
                throw std::runtime_error("Component type mismatch for MeshRenderer serialization");
            }
            nlohmann::json j;
            to_json(j, *meshRenderer);
            return j;
        },
        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto* meshRenderer = context.gameObject.addComponent<MeshRenderer>();
            from_json(context.json, *meshRenderer, context.assetManager);
        });
}
} // namespace dzemikk
#endif