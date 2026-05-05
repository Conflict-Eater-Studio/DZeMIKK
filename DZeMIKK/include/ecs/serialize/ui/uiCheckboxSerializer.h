#pragma once
#ifndef DZEMIKK_UICHECKBOXSERIALIZER_H
#define DZEMIKK_UICHECKBOXSERIALIZER_H

#include "ecs/components/ui/uiCheckbox.h"
#include "imageRendererSerializer.h"

#include <nlohmann/json.hpp>
namespace dzemikk {

    inline void to_json(nlohmann::json& json, const UICheckbox& button) {
        const auto normalColor = button.getNormalColor();
        const auto hoverColor = button.getHoverColor();
        const auto pressedColor = button.getPressedColor();

        json["type"] = button.typeName();
        json["id"] = boost::uuids::to_string(button.getId());
        json["normalColor"] = {normalColor[0], normalColor[1], normalColor[2], normalColor[3]};
        json["hoverColor"] = {hoverColor[0], hoverColor[1], hoverColor[2], hoverColor[3]};
        json["pressedColor"] = {pressedColor[0], pressedColor[1], pressedColor[2], pressedColor[3]};
        json["onClickActionId"] =
            button.getOnClickActionId().empty() ? "" : button.getOnClickActionId();
        json["onEnterActionId"] =
            button.getOnEnterActionId().empty() ? "" : button.getOnEnterActionId();
        json["onExitActionId"] = button.getOnExitActionId().empty() ? "" : button.getOnExitActionId();
        json["backgroundRenderer"] = nlohmann::json();
        dzemikk::to_json(json["backgroundRenderer"], *button.getBackgroundSpriteRenderer());
        json["checkmarkRenderer"] = nlohmann::json();
        dzemikk::to_json(json["checkmarkRenderer"], *button.getCheckmarkSpriteRenderer());
    }
    inline void from_json(const nlohmann::json& json, UICheckbox& checkbox, AssetManager* assetManager) {
        if (!json.contains("type") || !json["type"].is_string() || json["type"] != checkbox.typeName()) {
            throw std::runtime_error("Invalid component type for UICheckbox deserialization");
        }

        if (!json.contains("id")
            || !json.contains("colors") || !json["colors"].contains("normal")
            || !json["colors"].contains("hover") || !json["colors"].contains("pressed")
            || !json["colors"].contains("normal")
            || !json.contains("actions") || !json["actions"].contains("onClickId")
            || !json["actions"].contains("onClickId")
            || !json["actions"].contains("onEnterId")
            || !json["actions"].contains("onExitId")
            ) {
            throw std::runtime_error("Missing fields for UICheckbox deserialization");
        }

        if (json.contains("colors")) {
            auto c = json["colors"];
            checkbox.setNormalColor({c["normal"][0], c["normal"][1], c["normal"][2], c["normal"][3]});
            checkbox.setHoverColor({c["hover"][0], c["hover"][1], c["hover"][2], c["hover"][3]});
            checkbox.setPressedColor({c["pressed"][0], c["pressed"][1], c["pressed"][2], c["pressed"][3]});
        }

        if (json.contains("actions")) {
            auto a = json["actions"];
            checkbox.setOnClickActionId(a.value("onClickId", ""));
            checkbox.setOnEnterActionId(a.value("onEnterId", ""));
            checkbox.setOnExitActionId(a.value("onExitId", ""));
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
