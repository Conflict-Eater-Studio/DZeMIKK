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

        if (json.contains("id") && json["id"].is_string()) {
            try {
                renderer.setId(uuidGenerator(json["id"].get<std::string>()));
            } catch (const std::exception& e) {
    #if DZEMIKK_DEV_TOOLS
                spdlog::error("Exception parsing SkinnedMeshRenderer UUID: {}", e.what());
    #endif
            }
        }

        std::string modelPath = json.value("model", "");
        if (!modelPath.empty()) {
            renderer.setModel(assetManager->get<Model>(modelPath));
        }

        if (json.contains("materials") && json["materials"].is_array()) {
            const auto& materialsJson = json["materials"];
            for (size_t i = 0; i < materialsJson.size(); i++) {
                if (materialsJson[i].is_string()) {
                    std::string shaderPath = materialsJson[i].get<std::string>();
                    if (!shaderPath.empty()) {
                        try {
                            Material* material = new Material();
                            material->setShader(assetManager->get<Shader>(shaderPath));
                            renderer.setMaterial(i, material);
                        } catch (const std::exception& e) {
    #if DZEMIKK_DEV_TOOLS
                            spdlog::error("Exception loading material for SkinnedMeshRenderer: {}", e.what());
    #endif
                        }
                    }
                }
            }
        }
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