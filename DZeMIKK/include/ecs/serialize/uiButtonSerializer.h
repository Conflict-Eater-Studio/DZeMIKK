#ifndef DZEMIKK_UIBUTTONSERIALIZER_H
#define DZEMIKK_UIBUTTONSERIALIZER_H

#pragma once

#include "ecs/components/ui/uiButton.h"
#include "ecs/gameobject.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/serialize/uuid.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace dzemikk {
// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& json, const UIButton& button) {
    const auto normalColor = button.getNormalColor();
    const auto hoverColor = button.getHoverColor();
    const auto pressedColor = button.getPressedColor();

    json["type"] = "UIButton";
    json["id"] = boost::uuids::to_string(button.getId());
    json["normalColor"] = {normalColor[0], normalColor[1], normalColor[2], normalColor[3]};
    json["hoverColor"] = {hoverColor[0], hoverColor[1], hoverColor[2], hoverColor[3]};
    json["pressedColor"] = {pressedColor[0], pressedColor[1], pressedColor[2], pressedColor[3]};
    json["onClickActionId"] =
        button.getOnClickActionId().empty() ? "" : button.getOnClickActionId();
    json["onEnterActionId"] =
        button.getOnEnterActionId().empty() ? "" : button.getOnEnterActionId();
    json["onExitActionId"] = button.getOnExitActionId().empty() ? "" : button.getOnExitActionId();
}

inline void from_json(const nlohmann::json& json, UIButton& button) {
    static boost::uuids::string_generator uuidGenerator;

    if (!json.contains("type") || !json["type"].is_string() || json["type"] != "UIButton") {
        throw std::runtime_error("Invalid component type for UIButton deserialization");
    }

    if (!json.contains("id") || !json.contains("normalColor") || !json.contains("hoverColor") ||
        !json.contains("pressedColor")) {
        throw std::runtime_error("Missing required fields for UIButton deserialization");
    }

    const auto readVec4 = [&json](const char* fieldName) {
        const auto& value = json[fieldName];
        if (!value.is_array() || value.size() != 4) {
            throw std::runtime_error(std::string("Field '") + fieldName + "' must be vec4 array");
        }

        return glm::vec4(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(),
                         value[3].get<float>());
    };

    button.setId(uuidGenerator(json["id"].get<std::string>()));
    button.setNormalColor(readVec4("normalColor"));
    button.setHoverColor(readVec4("hoverColor"));
    button.setPressedColor(readVec4("pressedColor"));

    if (json.contains("onClickActionId") && json["onClickActionId"].is_string() &&
        json["onClickActionId"].get<std::string>() != "") {
        button.setOnClickActionId(json["onClickActionId"].get<std::string>());
    }
    if (json.contains("onEnterActionId") && json["onEnterActionId"].is_string() &&
        json["onEnterActionId"].get<std::string>() != "") {
        button.setOnEnterActionId(json["onEnterActionId"].get<std::string>());
    }
    if (json.contains("onExitActionId") && json["onExitActionId"].is_string() &&
        json["onExitActionId"].get<std::string>() != "") {
        button.setOnExitActionId(json["onExitActionId"].get<std::string>());
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
        [](GameObject& gameObject, const nlohmann::json& componentJson) {
            auto* button = gameObject.addComponent<UIButton>();
            from_json(componentJson, *button);
        });
}
// NOLINTEND(readability-identifier-naming)
} // namespace dzemikk

#endif // DZEMIKK_UIBUTTONSERIALIZER_H
