#ifndef DZEMIKK_UIBUTTONSERIALIZER_H
#define DZEMIKK_UIBUTTONSERIALIZER_H

#pragma once

#include "ecs/components/ui/uiButton.h"
#include "ecs/gameobject.h"
#include "ecs/serialize/componentSerializerRegistry.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace dzemikk {
// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& json, const UIButton& button) {
    const auto normalColor = button.getStyle().normalColor;
    const auto hoverColor = button.getStyle().hoverColor;
    const auto pressedColor = button.getStyle().pressedColor;

    json["type"] = "UIButton";
    json["id"] = boost::uuids::to_string(button.getId());
    json["normalColor"] = {normalColor[0], normalColor[1], normalColor[2], normalColor[3]};
    json["hoverColor"] = {hoverColor[0], hoverColor[1], hoverColor[2], hoverColor[3]};
    json["pressedColor"] = {pressedColor[0], pressedColor[1], pressedColor[2], pressedColor[3]};
    auto ea = button.getEventActions();
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

inline void from_json(const nlohmann::json& json, UIButton& button) {
    boost::uuids::string_generator uuidGenerator;

    if (!json.contains("type") || !json["type"].is_string() || json["type"] != "UIButton") {
        throw std::runtime_error("Invalid component type for UIButton deserialization");
    }

    if (!json.contains("id") || !json.contains("normalColor") || !json.contains("hoverColor") ||
        !json.contains("pressedColor") || !json.contains("events")) {
        throw std::runtime_error("Missing required fields for UIButton deserialization");
    }

    button.setId(uuidGenerator(json["id"].get<std::string>()));
    UIButton::Style style = {
        .normalColor =
            {
                json["normalColor"][0].get<float>(),
                json["normalColor"][1].get<float>(),
                json["normalColor"][2].get<float>(),
                json["normalColor"][3].get<float>(),
            },
        .hoverColor =
            {
                json["hoverColor"][0].get<float>(),
                json["hoverColor"][1].get<float>(),
                json["hoverColor"][2].get<float>(),
                json["hoverColor"][3].get<float>(),
            },
        .pressedColor =
            {
                json["pressedColor"][0].get<float>(),
                json["pressedColor"][1].get<float>(),
                json["pressedColor"][2].get<float>(),
                json["pressedColor"][3].get<float>(),
            },
    };
    button.setStyle(style);
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
            button.addEventListener(eventType, actionId);
        }
    }
}

inline void registerUIButtonSerializer(ComponentSerializerRegistry& registry) {
    registry.registerType(
        "UIButton",
        [](const Component& component) {
            const auto* button = dynamic_cast<const UIButton*>(&component);
            if (button == nullptr) {
                throw std::runtime_error("Component type mismatch for UIButton serialization");
            }

            return nlohmann::json(*button);
        },
        [](ComponentSerializerRegistry::DeserializationContext context) {
            auto* button = context.gameObject.addComponent<UIButton>();
            from_json(context.json, *button);
        });
}
// NOLINTEND(readability-identifier-naming)
} // namespace dzemikk

#endif // DZEMIKK_UIBUTTONSERIALIZER_H