#pragma once
#ifndef DZEMIKK_COLLIDERSERIALIZER_H
#define DZEMIKK_COLLIDERSERIALIZER_H

#include "ecs/components/collider.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "assetManager/assetmanager.h"

#include <nlohmann/json.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

namespace dzemikk {

inline void to_json(nlohmann::json& json, const Collider& collider) {
    json["type"] = collider.typeName();
    json["id"] = boost::uuids::to_string(collider.getId());

    if (collider.getModelHandle().get() != nullptr) {
        json["model"] = collider.getModelHandle().getAssetPath();
    } else {
        json["model"] = "";
    }
    const auto& pos = collider.getTransform()->getPosition();
    const auto& scale = collider.getTransform()->getScale();
    const auto& rot = collider.getTransform()->getRotation();

    json["position"] = { pos.x, pos.y, pos.z };
    json["rotation"] = { rot.w, rot.x, rot.y, rot.z };
    json["scale"] = { scale.x, scale.y, scale.z };
}

inline void from_json(const nlohmann::json& json, Collider& collider, AssetManager& assetManager) {
    static boost::uuids::string_generator uuidGenerator;

    if (json.contains("id") && json["id"].is_string()) {
        try {
            collider.setId(uuidGenerator(json["id"].get<std::string>()));
        } catch (const std::exception& e) {
#if DZEMIKK_DEV_TOOLS
            spdlog::warn("Malformed UUID in Collider JSON: {}", e.what());
#endif
        }
    }

    std::string modelPath = json.value("model", "");
    if (!modelPath.empty()) {
        collider.setModel(assetManager.get<Model>(modelPath));
    } else {
#if DZEMIKK_DEV_TOOLS
        spdlog::debug("Collider initialized without a model path.");
#endif
    }
}

inline void registerColliderSerializer(ComponentSerializerRegistry& registry, AssetManager& assetManager) {
    registry.registerType(
        "Collider",
        [](const Component& component) {
            const auto* collider = dynamic_cast<const Collider*>(&component);
            if (collider == nullptr) {
                throw std::runtime_error("Component type mismatch for Collider serialization");
            }
            nlohmann::json j;
            to_json(j, *collider);
            return j;
        },
        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto* collider = context.gameObject.addComponent<Collider>();
            from_json(context.json, *collider, context.assetManager);
        });
}

}
#endif