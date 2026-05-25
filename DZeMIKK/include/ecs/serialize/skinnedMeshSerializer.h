#pragma once
#ifndef DZEMIKK_SKINNEDMESHRENDERERSERIALIZER_H
#define DZEMIKK_SKINNEDMESHRENDERERSERIALIZER_H

#include "ecs/components/skinnedMeshRenderer.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "assetManager/assetmanager.h"

#include <nlohmann/json.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

namespace dzemikk {
    inline void to_json(nlohmann::json& json, const SkinnedMeshRenderer& renderer) {
        json["type"] = renderer.typeName();
        json["id"] = boost::uuids::to_string(renderer.getId());

        if (renderer.getModel().get() != nullptr) {
            json["model"] = renderer.getModel().getAssetPath();
        } else {
            json["model"] = "";
        }

        json["materials"] = nlohmann::json::array();
        for (const Material* material : renderer.getMaterials()) {
            if (material != nullptr && material->getShaderHandle().get() != nullptr) {
                json["materials"].push_back(material->getShaderHandle().getAssetPath());
            } else {
                json["materials"].push_back("");
            }
        }
    }

    inline void from_json(const nlohmann::json& json, SkinnedMeshRenderer& renderer, AssetManager* assetManager) {
        static boost::uuids::string_generator uuidGenerator;

        if (!json.contains("type") || !json["type"].is_string() || json["type"] != renderer.typeName()) {
            throw std::runtime_error("Invalid component type for SkinnedMeshRenderer deserialization");
        }

        if (!json.contains("id") || !json.contains("model") || !json.contains("materials") || !json["materials"].is_array()) {
            throw std::runtime_error("Missing fields for SkinnedMeshRenderer deserialization");

        }

        renderer.setId(uuidGenerator(json["id"].get<std::string>()));

        std::string modelPath = json.value("model", "");

        if (!modelPath.empty()) {

            constexpr std::string_view primitivePrefix = "primitive/";

            if (modelPath.starts_with(primitivePrefix)) {

                std::string primitiveIndexString = modelPath.substr(primitivePrefix.size());

                int primitiveIndex = std::stoi(primitiveIndexString);

                renderer.setModel(assetManager->getPrimitiveModel(
                    static_cast<PrimitiveMeshLibrary::PrimitiveMesh>(primitiveIndex)));

            }

            else {

                renderer.setModel(assetManager->get<Model>(modelPath));
            }
        }

        if (json.contains("materials") && json["materials"].is_array()) {
            const auto& materialsJson = json["materials"];
            for (size_t i = 0; i < materialsJson.size(); i++) {
                if (materialsJson[i].is_string()) {
                    std::string shaderPath = materialsJson[i].get<std::string>();
                    if (!shaderPath.empty()) {
                            Material* material = new Material();
                            material->setShader(assetManager->get<Shader>(shaderPath));
                            renderer.setMaterial(i, material);
                        }
                    }
                }
            }

        renderer.setTransform(renderer.getOwner()->transform());
    }

    inline void registerSkinnedMeshRendererSerializer(ComponentSerializerRegistry& registry) {
        registry.registerType(
            "SkinnedMeshRenderer",
            [](const Component& component) {
                const auto* renderer = dynamic_cast<const SkinnedMeshRenderer*>(&component);
                if (renderer == nullptr) {
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
}
#endif