#ifndef DZEMIKK_UIBUTTONSERIALIZER_H
#define DZEMIKK_UIBUTTONSERIALIZER_H

#pragma once

#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/uiButton.h"
#include "ecs/gameobject.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/serialize/ui/UITextRendererSerializer.h"
#include "ecs/serialize/ui/imageRendererSerializer.h"

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
    const auto disabledColor = button.getStyle().disabledColor;

    json["type"] = button.typeName();
    json["id"] = boost::uuids::to_string(button.getId());
    json["normalColor"] = {normalColor[0], normalColor[1], normalColor[2], normalColor[3]};
    json["hoverColor"] = {hoverColor[0], hoverColor[1], hoverColor[2], hoverColor[3]};
    json["pressedColor"] = {pressedColor[0], pressedColor[1], pressedColor[2], pressedColor[3]};
    json["disabledColor"] = {disabledColor[0], disabledColor[1], disabledColor[2],
                             disabledColor[3]};
    json["interactable"] = button.isInteractable();

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

inline void from_json(const nlohmann::json& json, UIButton& button, AssetManager* assetManager) {
    boost::uuids::string_generator uuidGenerator;

    if (!json.contains("type") || !json["type"].is_string() || json["type"] != "UIButton") {
        throw std::runtime_error("Invalid component type for UIButton deserialization");
    }

    if (!json.contains("id")) {
        throw std::runtime_error("Missing required fields for UIButton deserialization");
    }

    button.setId(uuidGenerator(json["id"].get<std::string>()));

    UIButton::Style style{};
    if (json.contains("normalColor")) {
        style.normalColor = {
            json["normalColor"][0].get<float>(), json["normalColor"][1].get<float>(),
            json["normalColor"][2].get<float>(), json["normalColor"][3].get<float>()};
    } else {
        style.normalColor = glm::vec4(1.0F);
    }

    if (json.contains("hoverColor")) {
        style.hoverColor = {json["hoverColor"][0].get<float>(), json["hoverColor"][1].get<float>(),
                            json["hoverColor"][2].get<float>(), json["hoverColor"][3].get<float>()};
    } else {
        style.hoverColor = glm::vec4(0.9F, 0.9F, 0.9F, 1.0F);
    }

    if (json.contains("pressedColor")) {
        style.pressedColor = {
            json["pressedColor"][0].get<float>(), json["pressedColor"][1].get<float>(),
            json["pressedColor"][2].get<float>(), json["pressedColor"][3].get<float>()};
    } else {
        style.pressedColor = glm::vec4(0.8F, 0.8F, 0.8F, 1.0F);
    }

    if (json.contains("disabledColor")) {
        style.disabledColor = {
            json["disabledColor"][0].get<float>(), json["disabledColor"][1].get<float>(),
            json["disabledColor"][2].get<float>(), json["disabledColor"][3].get<float>()};
    } else {
        style.disabledColor = glm::vec4(0.5F, 0.5F, 0.5F, 1.0F);
    }

    if (json.contains("interactable")) {
        button.setInteractable(json["interactable"].get<bool>());
    } else {
        button.setInteractable(true);
    }

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

    button.init(style, events);
}

inline void postUIButtonDeserialize(Component& component) {
    auto* button = dynamic_cast<UIButton*>(&component);
    if (button == nullptr) {
        throw std::runtime_error("Component type mismatch for UIButton post-deserialization");
    }

    button->applyVisualState();
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
            auto* uiButton = context.gameObject.addComponent<UIButton>();

            from_json(context.json, *uiButton, context.assetManager);
        },
        postUIButtonDeserialize);
}
// NOLINTEND(readability-identifier-naming)
} // namespace dzemikk

#endif // DZEMIKK_UIBUTTONSERIALIZER_H
