#pragma once
#ifndef DZEMIKK_UISLIDERSERIALIZER_H
#define DZEMIKK_UISLIDERSERIALIZER_H

#include "ecs/components/ui/uiSlider.h"
#include "ecs/gameobject.h"
#include "ecs/serialize/componentSerializerRegistry.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace dzemikk {
    inline void to_json(nlohmann::json& json, const UISlider& slider) {
        const auto fillColor = slider.getFillColor();
        const auto backgroundColor = slider.getBackgroundColor();
        const auto handleColor = slider.getHandleColor();
        const auto handleHoverColor = slider.getHandleHoverColor();
        const auto handlePressedColor = slider.getHandlePressedColor();

        json["type"] = "UISlider";
        json["id"] = boost::uuids::to_string(slider.getId());
        json["value"] = slider.getValue();
        json["fillColor"] = {fillColor[0], fillColor[1], fillColor[2], fillColor[3]};
        json["backgroundColor"] =
            {backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]};
        json["handleColor"] = {handleColor[0], handleColor[1], handleColor[2], handleColor[3]};
        json["handleHoverColor"] =
            {handleHoverColor[0], handleHoverColor[1], handleHoverColor[2], handleHoverColor[3]};
        json["handlePressedColor"] = {handlePressedColor[0], handlePressedColor[1],
                                       handlePressedColor[2], handlePressedColor[3]};
        json["onClickActionId"] =
            slider.getOnClickActionId().empty() ? "" : slider.getOnClickActionId();
        json["onEnterActionId"] =
            slider.getOnEnterActionId().empty() ? "" : slider.getOnEnterActionId();
        json["onExitActionId"] = slider.getOnExitActionId().empty() ? "" : slider.getOnExitActionId();
        json["onValueChangedActionId"] =
            slider.getOnValueChangedActionId().empty() ? "" : slider.getOnValueChangedActionId();
    }

    inline void from_json(const nlohmann::json& json, UISlider& slider) {
        static boost::uuids::string_generator uuidGenerator;

        if (!json.contains("type") || !json["type"].is_string() || json["type"] != "UISlider") {
            throw std::runtime_error("Invalid component type for UISlider deserialization");
        }

        if (!json.contains("id") || !json.contains("value") || !json.contains("fillColor") ||
            !json.contains("backgroundColor") || !json.contains("handleColor") ||
            !json.contains("handleHoverColor") || !json.contains("handlePressedColor")) {
            throw std::runtime_error("Missing required fields for UISlider deserialization");
        }

        const auto readVec4 = [&json](const char* fieldName) {
            const auto& value = json[fieldName];
            if (!value.is_array() || value.size() != 4) {
                throw std::runtime_error(std::string("Field '") + fieldName + "' must be vec4 array");
            }

            return glm::vec4(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(),
                             value[3].get<float>());
        };

        slider.setId(uuidGenerator(json["id"].get<std::string>()));
        slider.onValueChanged(json["value"].get<float>());
        slider.setFillColor(readVec4("fillColor"));
        slider.setBackgroundColor(readVec4("backgroundColor"));
        slider.setHandleColor(readVec4("handleColor"));
        slider.setHandleHoverColor(readVec4("handleHoverColor"));
        slider.setHandlePressedColor(readVec4("handlePressedColor"));

        if (json.contains("onClickActionId") && json["onClickActionId"].is_string() &&
            json["onClickActionId"].get<std::string>() != "") {
            slider.setOnClickActionId(json["onClickActionId"].get<std::string>());
        }
        if (json.contains("onEnterActionId") && json["onEnterActionId"].is_string() &&
            json["onEnterActionId"].get<std::string>() != "") {
            slider.setOnEnterActionId(json["onEnterActionId"].get<std::string>());
        }
        if (json.contains("onExitActionId") && json["onExitActionId"].is_string() &&
            json["onExitActionId"].get<std::string>() != "") {
            slider.setOnExitActionId(json["onExitActionId"].get<std::string>());
        }
        if (json.contains("onValueChangedActionId") && json["onValueChangedActionId"].is_string() &&
            json["onValueChangedActionId"].get<std::string>() != "") {
            slider.setOnValueChangedActionId(json["onValueChangedActionId"].get<std::string>());
        }
    }

    inline void registerUISliderSerializer(ComponentSerializerRegistry& registry) {
        registry.registerType(
            "UISlider",
            [](const Component& component) {
                const auto* slider = dynamic_cast<const UISlider*>(&component);
                if (slider == nullptr) {
                    throw std::runtime_error("Component type mismatch for UISlider serialization");
                }

                return nlohmann::json(*slider);
            },
            [](ComponentSerializerRegistry::DeserializationContext context) {
                auto* slider = context.gameObject.addComponent<UISlider>();
                from_json(context.json, *slider);
            });
    }
}

#endif