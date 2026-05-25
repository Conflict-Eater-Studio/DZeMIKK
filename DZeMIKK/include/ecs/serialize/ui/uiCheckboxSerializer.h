#pragma once
#ifndef DZEMIKK_UICHECKBOXSERIALIZER_H
#define DZEMIKK_UICHECKBOXSERIALIZER_H

#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/uiCheckbox.h"
#include "imageRendererSerializer.h"

#include <nlohmann/json.hpp>
namespace dzemikk {
// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& json, const UICheckbox& checkbox) {
    const auto normalColor = checkbox.getStyle().normalColor;
    const auto hoverColor = checkbox.getStyle().hoverColor;
    const auto pressedColor = checkbox.getStyle().pressedColor;
    const auto checkmarkColor = checkbox.getStyle().checkmarkColor;

    json["type"] = checkbox.typeName();
    json["id"] = boost::uuids::to_string(checkbox.getId());
    json["normalColor"] = {normalColor[0], normalColor[1], normalColor[2], normalColor[3]};
    json["hoverColor"] = {hoverColor[0], hoverColor[1], hoverColor[2], hoverColor[3]};
    json["pressedColor"] = {pressedColor[0], pressedColor[1], pressedColor[2], pressedColor[3]};
    json["checkmarkColor"] = {checkmarkColor[0], checkmarkColor[1], checkmarkColor[2],
                              checkmarkColor[3]};

    json["value"] = checkbox.getValue(); 

    auto ea = checkbox.getEventActions();
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
inline void from_json(const nlohmann::json& json, UICheckbox& checkbox,
                      AssetManager* assetManager) {
    if (!json.contains("type") || !json["type"].is_string() ||
        json["type"] != checkbox.typeName()) {
        throw std::runtime_error("Invalid component type for UICheckbox deserialization");
    }

    if (!json.contains("id") || !json.contains("normalColor") || !json.contains("hoverColor") ||
        !json.contains("pressedColor") || !json.contains("checkmarkColor")) {
        throw std::runtime_error("Missing fields for UICheckbox deserialization");
    }

    checkbox.setId(boost::uuids::string_generator()(json["id"].get<std::string>()));

    UICheckbox::Style style;
    style.normalColor = {json["normalColor"][0], json["normalColor"][1], json["normalColor"][2],
                         json["normalColor"][3]};
    style.hoverColor = {json["hoverColor"][0], json["hoverColor"][1], json["hoverColor"][2],
                        json["hoverColor"][3]};
    style.pressedColor = {json["pressedColor"][0], json["pressedColor"][1], json["pressedColor"][2],
                          json["pressedColor"][3]};
    style.checkmarkColor = {json["checkmarkColor"][0], json["checkmarkColor"][1],
                            json["checkmarkColor"][2], json["checkmarkColor"][3]};

    std::vector<std::pair<UIEventType, std::string>> events;
    if (json.contains("events")) {
        for (const auto& [eventKey, actionIdsJson] : json["events"].items()) {
            UIEventType eventType = UIEventType::Click;
            if (eventKey == "click") {
                eventType = UIEventType::Click;
            } else if (eventKey == "enter") {
                eventType = UIEventType::Enter;
            } else if (eventKey == "exit") {
                eventType = UIEventType::Exit;
            } else if (eventKey == "valueChanged") {
                eventType = UIEventType::ValueChanged;
            } else {
                continue;
            }
            std::vector<std::string> actionIds = actionIdsJson.get<std::vector<std::string>>();
            for (const auto& actionId : actionIds) {
                events.emplace_back(eventType, actionId);
            }
        }
    }

    checkbox.init(style, json.value("value", false), events);
}

inline void postUICheckboxDeserialize(Component& component) {
    auto* checkbox = dynamic_cast<UICheckbox*>(&component);
    if (checkbox == nullptr) {
        throw std::runtime_error("Component type mismatch for UICheckbox post-deserialization");
    }
    checkbox->applyVisualState();
}

inline void registerUICheckboxSerializer(ComponentSerializerRegistry& registry) {
    registry.registerType(
        "UICheckbox",
        [](const Component& component) {
            const auto* checkbox = dynamic_cast<const UICheckbox*>(&component);
            nlohmann::json j;
            to_json(j, *checkbox);
            return j;
        },
        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto* checkbox = context.gameObject.addComponent<UICheckbox>();
            from_json(context.json, *checkbox, context.assetManager);
        },
        postUICheckboxDeserialize);
}
// NOLINTEND(readability-identifier-naming)
} // namespace dzemikk
#endif
