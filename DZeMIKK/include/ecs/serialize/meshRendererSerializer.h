#pragma once
#ifndef DZEMIKK_MESHRENDERERSERIALIZER_H
#define DZEMIKK_MESHRENDERERSERIALIZER_H

#include "ecs/components/meshRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/serialize/uuid.h"
#include "ecs/components/transform.h"
#include "renderer/model.h"
#include "renderer/shader.h"
#include "renderer/material.h"

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
        if (material != nullptr) {
            const auto& shader = material->getShaderHandle();
            if (shader.get() != nullptr) {
                json["materials"].push_back(shader.getAssetPath());
            } else {
                json["materials"].push_back("");
            }
        } else {
            json["materials"].push_back("");
        }
    }
}

inline void from_json(const nlohmann::json& json, MeshRenderer& meshRenderer, AssetManager* assetManager) {
    static boost::uuids::string_generator uuidGenerator;

    if (!json.contains("type") || !json["type"].is_string() || json["type"] != meshRenderer.typeName()) {
        throw std::runtime_error("Invalid component type for MeshRenderer deserialization");
    }

    if (!json.contains("id") || !json.contains("model") || !json.contains("color") || !json.contains("materials") || !json["materials"].is_array()) {
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

        const auto& materialsJson = json["materials"];

        for (size_t i = 0; i < materialsJson.size(); i++) {

            if (materialsJson[i].is_string()) {

                std::string shaderPath = materialsJson[i].get<std::string>();

                if (!shaderPath.empty()) {

                    auto material = std::make_shared<Material>();

                    material->setShader(assetManager->get<Shader>(shaderPath));

                    meshRenderer.setMaterial(i, material);
                }
            }
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
}
#endif
