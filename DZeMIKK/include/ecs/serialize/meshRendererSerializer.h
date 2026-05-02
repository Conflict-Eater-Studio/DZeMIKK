#ifndef DZEMIKK_MESHRENDERERSERIALIZER_H
#define DZEMIKK_MESHRENDERERSERIALIZER_H

#pragma once

#include "ecs/components/meshRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/serialize/uuid.h"
#include "renderer/shader.h"

#include <nlohmann/json.hpp>

namespace dzemikk {
inline void to_json(nlohmann::json& json, const MeshRenderer& meshRenderer) {
    json["type"] = meshRenderer.typeName();
    json["id"] = meshRenderer.getId();

    if (meshRenderer.getModelHandle().get() != nullptr) {
        json["model"] = meshRenderer.getModelHandle().getAssetPath();
    }else {
        json["model"] = "";
    }

    json["materials"] = nlohmann::json::array();
    for (const auto& material : meshRenderer.getMaterials()) {
        json["materials"].push_back("");
    }

    const auto& color = meshRenderer.getColor();
    json["color"] = { color.r, color.g, color.b, color.a };
}


inline void from_json(const nlohmann::json& json, MeshRenderer& meshRenderer) {
    // static boost::uuids::string_generator uuidGenerator;
    // meshRenderer.setId(uuidGenerator(json["id"]));
    //
    // std::string modelPath = json.value("model", "");
    //
    // if (json.contains("color")) {
    //     auto c = json["color"];
    //     meshRenderer.setColor(glm::vec4(c[0], c[1], c[2], c[3]));
    // }
}

inline void registerMeshRendererSerializer(ComponentSerializerRegistry& registry) {
    registry.registerType(
        "MeshRenderer",
        [](const Component& component) {
            const auto* meshRenderer = dynamic_cast<const MeshRenderer*>(&component);
            if (meshRenderer == nullptr) {
                throw std::runtime_error("Component type mismatch for MeshRenderer serialization");
            }
            return nlohmann::json(*meshRenderer);
        },
        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto* meshRenderer = context.gameObject.addComponent<MeshRenderer>();
            from_json(context.json, *meshRenderer);
        });
}
}
#endif
