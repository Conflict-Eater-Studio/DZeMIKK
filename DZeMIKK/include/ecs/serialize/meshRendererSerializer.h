#pragma once
#ifndef DZEMIKK_MESHRENDERERSERIALIZER_H
#define DZEMIKK_MESHRENDERERSERIALIZER_H

#include "ecs/components/meshRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/serialize/uuid.h"
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

    for (const Material* material : materials) {
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

    const auto& color = meshRenderer.getColor();
    json["color"] = { color.r, color.g, color.b, color.a };
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
        meshRenderer.setModel(assetManager->get<Model>(modelPath));
    }

    if (json.contains("color") && json["color"].is_array() && json["color"].size() >= 4) {
        const auto& c = json["color"];
        meshRenderer.setColor(glm::vec4(
            c[0].get<float>(),
            c[1].get<float>(),
            c[2].get<float>(),
            c[3].get<float>()
        ));
    }

    if (json.contains("materials") && json["materials"].is_array()) {
        const auto& materialsJson = json["materials"];
        for (size_t i = 0; i < materialsJson.size(); i++) {
            if (materialsJson[i].is_string()) {
                std::string shaderPath = materialsJson[i].get<std::string>();
                if (!shaderPath.empty()) {
                    Material* material = new Material();
                    material->setShader(assetManager->get<Shader>(shaderPath));
                    meshRenderer.setMaterial(i, material);
                }
            }
        }
    }
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
