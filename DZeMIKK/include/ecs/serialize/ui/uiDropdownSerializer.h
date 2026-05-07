#ifndef DZEMIKK_UIDROPDOWNSERIALIZER_H
#define DZEMIKK_UIDROPDOWNSERIALIZER_H
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/components/ui/uiDropdown.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/serialize/ui/imageRendererSerializer.h"
#include <nlohmann/json.hpp>

namespace dzemikk {

inline void to_json(nlohmann::json& json, const UIDropdown& dropdown) {
    json["type"] = dropdown.typeName();
    json["id"] = boost::uuids::to_string(dropdown.getId());

    json["selectedIndex"] = dropdown.getSelectedIndex();

    nlohmann::json optionsArray = nlohmann::json::array();
    for (const auto& opt : dropdown.getOptions()) {
        nlohmann::json optionJson;
        optionJson["text"] = opt.text;
        optionJson["value"] = opt.value;
        optionsArray.push_back(optionJson);
    }
    json["options"] = optionsArray;

    const auto& style = dropdown.getStyle();
    json["normalColor"] = { style.normalColor[0], style.normalColor[1], style.normalColor[2], style.normalColor[3] };
    json["hoverColor"] = { style.hoverColor[0], style.hoverColor[1], style.hoverColor[2], style.hoverColor[3] };
    json["pressedColor"] = { style.pressedColor[0], style.pressedColor[1], style.pressedColor[2], style.pressedColor[3] };
    json["arrowColor"] = { style.arrowColor[0], style.arrowColor[1], style.arrowColor[2], style.arrowColor[3] };
    json["textColor"] = { style.textColor[0], style.textColor[1], style.textColor[2], style.textColor[3] };
    json["normalOptColor"] = { style.normalOptColor[0], style.normalOptColor[1], style.normalOptColor[2], style.normalOptColor[3] };
    json["hoverOptColor"] = { style.hoverOptColor[0], style.hoverOptColor[1], style.hoverOptColor[2], style.hoverOptColor[3] };
    json["pressedOptColor"] = { style.pressedOptColor[0], style.pressedOptColor[1], style.pressedOptColor[2], style.pressedOptColor[3] };
    json["highlightOptColor"] = { style.highlightOptColor[0], style.highlightOptColor[1], style.highlightOptColor[2], style.highlightOptColor[3] };

    const auto& r = dropdown.getOptionRender();
    json["optionRender"]["height"] = r.height;
    json["optionRender"]["textVAlign"] = r.textHAlign;
    json["optionRender"]["textHAlign"] = r.textHAlign;

    if (r.font) {
        json["optionRender"]["fontPath"] = r.font.getAssetPath();
    }else {
        json["optionRender"]["fontPath"] = "";
    }

    json["renderers"] = nlohmann::json::array();
    to_json(json["renderers"]["backgroundSpriteRenderer"], dropdown.getBackgroundSpriteRenderer());
    to_json(json["renderers"]["arrowSpriteRenderer"], dropdown.getArrowSpriteRenderer());
    to_json(json["renderers"]["optionsBackgroundSpriteRenderer"], dropdown.getOptionsBackgroundRenderer());
    to_json(json["renderers"]["scrollbarSpriteRenderer"], dropdown.getScrollbarSpriteRenderer());
    to_json(json["renderers"]["scrollbarHandleSpriteRenderer"], dropdown.getScrollbarHandleSpriteRenderer());

}

inline void from_json(const nlohmann::json& json, UIDropdown& dropdown, AssetManager* assetManger) {
    if (!json.contains("type") || json["type"] != dropdown.typeName()) {
        throw std::runtime_error("Invalid type for UIDropdown deserialization");
    }

    if (!json.contains("id") || !json.contains("normalColor") || !json.contains("arrowColor") || !json.contains("textColor") ||
            !json.contains("hoverColor") || !json.contains("pressedColor") || !json.contains("normalOptColor") || !json.contains("hoverOptColor") ||
            !json.contains("options") || !json.contains("selectedIndex") || !json.contains("pressedOptColor") || !json.contains("highlightOptColor")) {
        throw std::runtime_error("Missing critical fields for UIDropdown deserialization");
    }

    dropdown.setId(boost::uuids::string_generator()(json.at("id").get<std::string>()));

    std::vector<UIDropdown::Option> opts;
    for (const auto& optionJson : json["options"]) {
        opts.push_back({optionJson["text"], optionJson["value"]});
    }
    dropdown.setOptions(opts);

    dropdown.setSelectedIndex(json["selectedIndex"]);

    UIDropdown::Style style;
    style.normalColor = {json["normalColor"][0], json["normalColor"][1], json["normalColor"][2], json["normalColor"][3]};
    style.hoverColor = {json["hoverColor"][0], json["hoverColor"][1], json["hoverColor"][2], json["hoverColor"][3]};
    style.pressedColor = {json["pressedColor"][0], json["pressedColor"][1], json["pressedColor"][2], json["pressedColor"][3]};
    style.arrowColor = {json["arrowColor"][0], json["arrowColor"][1], json["arrowColor"][2], json["arrowColor"][3]};
    style.textColor = {json["textColor"][0], json["textColor"][1], json["textColor"][2], json["textColor"][3]};
    style.normalOptColor = {json["normalOptColor"][0], json["normalOptColor"][1], json["normalOptColor"][2], json["normalOptColor"][3]};
    style.hoverOptColor = {json["hoverOptColor"][0], json["hoverOptColor"][1], json["hoverOptColor"][2], json["hoverOptColor"][3]};
    style.pressedOptColor = {json["pressedOptColor"][0], json["pressedOptColor"][1], json["pressedOptColor"][2], json["pressedOptColor"][3]};
    style.highlightOptColor = {json["highlightOptColor"][0], json["highlightOptColor"][1], json["highlightOptColor"][2], json["highlightOptColor"][3]};
    dropdown.setStyle(style);

    // 4. Capture Component References (Resolution happens in second pass)
    auto stringToUuid = [](const nlohmann::json& j) {
        return boost::uuids::string_generator()(j.get<std::string>());
    };

    if (json.contains("backgroundId")) dropdown.getBackgroundRef().captureSerializedId(stringToUuid(json["backgroundId"]));
    if (json.contains("arrowId"))      dropdown.getArrowRef().captureSerializedId(stringToUuid(json["arrowId"]));
    if (json.contains("optionsBgId"))  dropdown.getOptionsBgRef().captureSerializedId(stringToUuid(json["optionsBgId"]));

    auto r = dropdown.getOptionRender();
    const auto& rj = json["optionRender"];
    r.height = rj["height"];
    r.textHAlign = rj["textHAlign"];
    r.textVAlign = rj["textVAlign"];

    if (rj.contains("fontPath")) {
        r.font = assetManger->get<Font>(rj["fontPath"]);
    }

    auto backgroundSpriteRenderer = dropdown.getOwner()->addComponent<ImageRenderer>();
    auto arrowSpriteRenderer = dropdown.getOwner()->addComponent<ImageRenderer>();
    auto optionsBackgroundRenderer = dropdown.getOwner()->addComponent<ImageRenderer>();
    auto scrollbarSpriteRenderer = dropdown.getOwner()->addComponent<ImageRenderer>();
    auto scrollbarHandleSpriteRenderer = dropdown.getOwner()->addComponent<ImageRenderer>();

    from_json(json["renderers"]["backgroundSpriteRenderer"], *backgroundSpriteRenderer, assetManger);
    from_json(json["renderers"]["arrowSpriteRenderer"], *arrowSpriteRenderer, assetManger);
    from_json(json["renderers"]["optionsBackgroundSpriteRenderer"], *optionsBackgroundRenderer, assetManger);
    from_json(json["renderers"]["scrollbarSpriteRenderer"], *scrollbarSpriteRenderer, assetManger);
    from_json(json["renderers"]["scrollbarHandleSpriteRenderer"], *scrollbarHandleSpriteRenderer, assetManger);

    dropdown.setBackgroundSpriteRenderer(backgroundSpriteRenderer);
    dropdown.setArrowSpriteRenderer(arrowSpriteRenderer);
    dropdown.setOptionsBackgroundRenderer(optionsBackgroundRenderer);
    dropdown.setScrollbarSpriteRenderer(scrollbarSpriteRenderer);
    dropdown.setScrollbarHandleSpriteRenderer(scrollbarHandleSpriteRenderer);
    dropdown.setOptionRender(r);

    dropdown.applyVisualState();
}

inline void registerUIDropdownSerializer(ComponentSerializerRegistry& registry) {
    registry.registerType(
        "UIDropdown",
        [](const Component& component) {
            const auto* checkbox = dynamic_cast<const UIDropdown*>(&component);
            nlohmann::json j;
            to_json(j, *checkbox);
            return j;
        },
        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto* dropdown = context.gameObject.addComponent<UIDropdown>();
            from_json(context.json, *dropdown);
        });
}

} // namespace dzemikk
#endif
