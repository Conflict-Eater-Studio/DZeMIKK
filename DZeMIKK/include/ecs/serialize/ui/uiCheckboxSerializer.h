#pragma once
#ifndef DZEMIKK_UICHECKBOXSERIALIZER_H
#define DZEMIKK_UICHECKBOXSERIALIZER_H

#include "ecs/components/ui/uiCheckbox.h"
#include "imageRendererSerializer.h"

#include <nlohmann/json.hpp>
namespace dzemikk {
    inline void to_json(nlohmann::json& json, const UICheckbox& checkbox) {
        const auto normalColor = checkbox.getStyle().normalColor;
        const auto hoverColor = checkbox.getStyle().hoverColor;
        const auto pressedColor = checkbox.getStyle().pressedColor;
        const auto checkmarkColor = checkbox.getStyle().checkmarkColor;
        const auto& ea = checkbox.getEventActions();

        json["type"] = checkbox.typeName();
        json["id"] = boost::uuids::to_string(checkbox.getId());
        json["normalColor"] = {normalColor[0], normalColor[1], normalColor[2], normalColor[3]};
        json["hoverColor"] = {hoverColor[0], hoverColor[1], hoverColor[2], hoverColor[3]};
        json["pressedColor"] = {pressedColor[0], pressedColor[1], pressedColor[2], pressedColor[3]};
        json["checkmarkColor"] = {checkmarkColor[0], checkmarkColor[1], checkmarkColor[2], checkmarkColor[3]};

        for (const auto& [eventType, actionIds] : checkbox.getEventActions()) {
            const char* eventTypeStr = nullptr;
            switch (eventType) {
            case UIEventType::Click:
                eventTypeStr = "clickActions";
                break;
            case UIEventType::Enter:
                eventTypeStr = "enterActions";
                break;
            case UIEventType::Exit:
                eventTypeStr = "exitActions";
                break;
            case UIEventType::ValueChanged:
                eventTypeStr = "valueChangedActions";
                break;
            default:
                continue;
            }
            json[eventTypeStr] = actionIds;
        }
        json["backgroundRenderer"] = nlohmann::json();
        dzemikk::to_json(json["backgroundRenderer"], *checkbox.getBackgroundSpriteRenderer());
        json["checkmarkRenderer"] = nlohmann::json();
        dzemikk::to_json(json["checkmarkRenderer"], *checkbox.getCheckmarkSpriteRenderer());
    }
    inline void from_json(const nlohmann::json& json, UICheckbox& checkbox, AssetManager* assetManager) {
        if (!json.contains("type") || !json["type"].is_string() || json["type"] != checkbox.typeName()) {
            throw std::runtime_error("Invalid component type for UICheckbox deserialization");
        }

        if (!json.contains("id")
            || !json.contains("normalColor")
            || !json.contains("hoverColor")
            || !json.contains("pressedColor")
            || !json.contains("checkmarkColor")
            ) {
            throw std::runtime_error("Missing fields for UICheckbox deserialization");
        }

        checkbox.setId(boost::uuids::string_generator()(json["id"].get<std::string>()));

        UICheckbox::Style style;
        style.normalColor = {json["normalColor"][0], json["normalColor"][1], json["normalColor"][2], json["normalColor"][3]};
        style.hoverColor = {json["hoverColor"][0], json["hoverColor"][1], json["hoverColor"][2], json["hoverColor"][3]};
        style.pressedColor = {json["pressedColor"][0], json["pressedColor"][1], json["pressedColor"][2], json["pressedColor"][3]};
        style.checkmarkColor = {json["checkmarkColor"][0], json["checkmarkColor"][1], json["checkmarkColor"][2], json["checkmarkColor"][3]};
        checkbox.setStyle(style);

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
                checkbox.addEventListener(eventType, actionId);
            }
        }

        auto* background = checkbox.getOwner()->addComponent<ImageRenderer>();
        auto* checkmark = checkbox.getOwner()->addComponent<ImageRenderer>();
        dzemikk::from_json(json["backgroundRenderer"], *background, assetManager);
        dzemikk::from_json(json["checkmarkRenderer"], *checkmark, assetManager);
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
            });
    }
}
#endif
