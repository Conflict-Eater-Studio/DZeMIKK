#pragma once
#ifndef DZEMIKK_RECTTRANSFORMSERIALIZER_H
#define DZEMIKK_RECTTRANSFORMSERIALIZER_H

#include "ecs/components/ui/rectTransform.h"
#include "ecs/gameobject.h"
#include "ecs/serialize/componentSerializerRegistry.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace dzemikk {
    inline void to_json(nlohmann::json& json, const RectTransform& trs) {
        auto pos = trs.getPosition();
        auto scale = trs.getScale();
        auto anchorMin = trs.getAnchorMin();
        auto anchorMax = trs.getAnchorMax();
        auto pivot = trs.getPivot();
        auto rot = trs.getRotation();
        auto size = trs.getSize();
        auto zIndex = trs.getZIndex();

        json["type"] = trs.typeName();
        json["id"] = boost::uuids::to_string(trs.getId());
        json["position"] = {pos[0], pos[1]};
        json["rotation"] = rot;
        json["scale"] = {scale[0], scale[1]};
        json["anchorMin"] = {anchorMin[0], anchorMin[1]};
        json["anchorMax"] = {anchorMax[0], anchorMax[1]};
        json["pivot"] = {pivot[0], pivot[1]};
        json["size"] = {size[0], size[1]};
        json["zIndex"] = zIndex;
    }

    inline void from_json(const nlohmann::json& json, RectTransform& trs) {
        static boost::uuids::string_generator uuidGenerator;

        if (!json.contains("type") || !json["type"].is_string() || json["type"] != "RectTransform") {
            throw std::runtime_error("Invalid component type for RectTransform deserialization");
        }

        auto keys = {"id",        "position", "rotation", "scale", "anchorMin",
                     "anchorMax", "pivot",    "size",     "zIndex"};

        if (!std::ranges::all_of(keys, [&json](const char* key) { return json.contains(key); })) {
            throw std::runtime_error("Missing required fields for RectTransform deserialization");
        }

        const auto& pos = json["position"];
        const auto& rot = json["rotation"];
        const auto& scale = json["scale"];
        const auto& anchorMin = json["anchorMin"];
        const auto& anchorMax = json["anchorMax"];
        const auto& pivot = json["pivot"];
        const auto& size = json["size"];
        const auto& zIndex = json["zIndex"];

        trs.setId(uuidGenerator(json["id"].get<std::string>()));
        trs.setPosition({pos[0].get<float>(), pos[1].get<float>()});
        trs.setRotation(rot.get<float>());
        trs.setScale({scale[0].get<float>(), scale[1].get<float>()});
        trs.setAnchorMin({anchorMin[0].get<float>(), anchorMin[1].get<float>()});
        trs.setAnchorMax({anchorMax[0].get<float>(), anchorMax[1].get<float>()});
        trs.setPivot({pivot[0].get<float>(), pivot[1].get<float>()});
        trs.setSize({size[0].get<float>(), size[1].get<float>()});
        trs.setZIndex(zIndex.get<unsigned int>());
    }

    inline void registerRectTransformSerializer(ComponentSerializerRegistry& registry) {

        registry.registerType(
            "RectTransform",

            [](const Component& component) {
                const auto* transform = dynamic_cast<const RectTransform*>(&component);

                if (transform == nullptr) {
                    throw std::runtime_error(
                        "Component type mismatch for RectTransform serialization");
                }
                return nlohmann::json(*transform);
            },

            [](ComponentSerializerRegistry::DeserializationContext context) {
                if (!context.gameObject.getComponent<dzemikk::RectTransform>()) {
                    context.gameObject.replaceTransformWithRectTransform();
                }

                auto* rectTransform = context.gameObject.rectTransform();

                if (!rectTransform) {
                    throw std::runtime_error(
                        "Failed to create RectTransform during deserialization");
                }

                from_json(context.json, *rectTransform);
            });
    }
    }
#endif