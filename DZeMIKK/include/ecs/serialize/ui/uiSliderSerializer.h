#ifndef DZEMIKK_UISLIDERSERIALIZER_H
#define DZEMIKK_UISLIDERSERIALIZER_H

#pragma once

#include "ecs/components/ui/uiSlider.h"
#include "ecs/gameobject.h"
#include "ecs/serialize/componentSerializerRegistry.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace dzemikk {
// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& json, const UISlider& slider) {
    const auto style = slider.getStyle();

    json["type"] = slider.typeName();
    json["id"] = boost::uuids::to_string(slider.getId());
    json["value"] = slider.getValue();
    json["minValue"] = slider.getMinValue();
    json["maxValue"] = slider.getMaxValue();
    json["step"] = slider.getStep();
    json["fillColor"] = {style.fillColor[0], style.fillColor[1], style.fillColor[2],
                         style.fillColor[3]};
    json["backgroundColor"] = {style.backgroundColor[0], style.backgroundColor[1],
                               style.backgroundColor[2], style.backgroundColor[3]};
    json["handleColor"] = {style.handleColor[0], style.handleColor[1], style.handleColor[2],
                           style.handleColor[3]};
    json["handleHoverColor"] = {style.handleHoverColor[0], style.handleHoverColor[1],
                                style.handleHoverColor[2], style.handleHoverColor[3]};
    json["handlePressedColor"] = {style.handlePressedColor[0], style.handlePressedColor[1],
                                  style.handlePressedColor[2], style.handlePressedColor[3]};

    auto ea = slider.getEventActions();
    for (const auto& [eventType, actionIds] : ea) {
        const char* eventKey = nullptr;
        switch (eventType) {
        case UIEventType::Click:
            eventKey = "click";
            break;
        case UIEventType::Enter:
            eventKey = "enter";
            break;
        case UIEventType::Exit:
            eventKey = "exit";
            break;
        case UIEventType::ValueChanged:
            eventKey = "valueChanged";
            break;
        default:
            continue;
        }
        json["events"][eventKey] = actionIds;
    }
}

inline void from_json(const nlohmann::json& json, UISlider& slider) {
    boost::uuids::string_generator uuidGenerator;

    if (!json.contains("type") || !json["type"].is_string() || json["type"] != "UISlider") {
        throw std::runtime_error("Invalid component type for UISlider deserialization");
    }

    if (!json.contains("id") || !json.contains("value") || !json.contains("fillColor") ||
        !json.contains("backgroundColor") || !json.contains("handleColor") ||
        !json.contains("handleHoverColor") || !json.contains("handlePressedColor") ||
        !json.contains("events")) {
        throw std::runtime_error("Missing required fields for UISlider deserialization");
    }

    slider.setId(uuidGenerator(json["id"].get<std::string>()));
    slider.setMinValue(json["minValue"]);
    slider.setMaxValue(json["maxValue"]);
    slider.setStep(json["step"]);
    slider.onValueChanged(json["value"]);

    UISlider::Style style;
    style.fillColor = {json["fillColor"][0], json["fillColor"][1], json["fillColor"][2],
                       json["fillColor"][3]};
    style.backgroundColor = {json["backgroundColor"][0], json["backgroundColor"][1],
                             json["backgroundColor"][2], json["backgroundColor"][3]};
    style.handleColor = {json["handleColor"][0], json["handleColor"][1], json["handleColor"][2],
                         json["handleColor"][3]};
    style.handleHoverColor = {json["handleHoverColor"][0], json["handleHoverColor"][1],
                              json["handleHoverColor"][2], json["handleHoverColor"][3]};
    style.handlePressedColor = {json["handlePressedColor"][0], json["handlePressedColor"][1],
                                json["handlePressedColor"][2], json["handlePressedColor"][3]};

    slider.setStyle(style);
    for (const auto& [eventKey, actionIdsJson] : json["events"].items()) {
        UIEventType eventType = UIEventType::Click;
        if (eventKey == "clickActions") {
            eventType = UIEventType::Click;
        } else if (eventKey == "enterActions") {
            eventType = UIEventType::Enter;
        } else if (eventKey == "exitActions") {
            eventType = UIEventType::Exit;
        } else if (eventKey == "valueChangedActions") {
            eventType = UIEventType::ValueChanged;
        } else {
            continue;
        }
        std::vector<std::string> actionIds = actionIdsJson.get<std::vector<std::string>>();
        for (const auto& actionId : actionIds) {
            slider.addEventListener(eventType, actionId);
        }
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
// NOLINTEND(readability-identifier-naming)
} // namespace dzemikk

#endif // DZEMIKK_UISLIDERSERIALIZER_H
