#ifndef DZEMIKK_UIDROPDOWNSERIALIZER_H
#define DZEMIKK_UIDROPDOWNSERIALIZER_H
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/uiDropdown.h"
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/serialize/ui/imageRendererSerializer.h"

#include <nlohmann/json.hpp>

namespace dzemikk {
inline void to_json(nlohmann::json& json, const UIDropdown& dropdown) {
    json["type"] = dropdown.typeName();
    json["id"] = boost::uuids::to_string(dropdown.getId());

    if (dropdown.getSelectedIndex() != UIDropdown::noSelection()) {
        json["selectedIndex"] = dropdown.getSelectedIndex();
    }
    json["triggerActionId"] = boost::uuids::to_string(dropdown.getTriggerActionId());

    nlohmann::json optionsArray = nlohmann::json::array();
    for (const auto& opt : dropdown.getOptions()) {
        nlohmann::json optionJson;
        optionJson["text"] = opt.text;
        optionJson["value"] = opt.value;
        optionsArray.push_back(optionJson);
    }
    json["options"] = optionsArray;

    const auto& style = dropdown.getStyle();
    json["normalColor"] = {style.normalColor[0], style.normalColor[1], style.normalColor[2],
                           style.normalColor[3]};
    json["hoverColor"] = {style.hoverColor[0], style.hoverColor[1], style.hoverColor[2],
                          style.hoverColor[3]};
    json["pressedColor"] = {style.pressedColor[0], style.pressedColor[1], style.pressedColor[2],
                            style.pressedColor[3]};
    json["arrowColor"] = {style.arrowColor[0], style.arrowColor[1], style.arrowColor[2],
                          style.arrowColor[3]};
    json["textColor"] = {style.textColor[0], style.textColor[1], style.textColor[2],
                         style.textColor[3]};
    json["normalOptColor"] = {style.normalOptColor[0], style.normalOptColor[1],
                              style.normalOptColor[2], style.normalOptColor[3]};
    json["hoverOptColor"] = {style.hoverOptColor[0], style.hoverOptColor[1], style.hoverOptColor[2],
                             style.hoverOptColor[3]};
    json["pressedOptColor"] = {style.pressedOptColor[0], style.pressedOptColor[1],
                               style.pressedOptColor[2], style.pressedOptColor[3]};
    json["highlightOptColor"] = {style.highlightOptColor[0], style.highlightOptColor[1],
                                 style.highlightOptColor[2], style.highlightOptColor[3]};

    const auto& r = dropdown.getOptionRender();
    json["optionRender"]["height"] = r.height;
    json["optionRender"]["textVAlign"] = r.textVAlign;
    json["optionRender"]["textHAlign"] = r.textHAlign;
    json["optionRender"]["font"] = r.font.getAssetPath();
    json["optionRender"]["materialPath"] = r.material->getShaderHandle().getAssetPath();
}

inline void from_json(const nlohmann::json& json, UIDropdown& dropdown, AssetManager* assetManger) {
    if (!json.contains("type") || json["type"] != dropdown.typeName()) {
        throw std::runtime_error("Invalid type for UIDropdown deserialization");
    }

    if (!json.contains("id") || !json.contains("normalColor") || !json.contains("arrowColor") ||
        !json.contains("textColor") || !json.contains("hoverColor") ||
        !json.contains("pressedColor") || !json.contains("normalOptColor") ||
        !json.contains("hoverOptColor") || !json.contains("options") ||
        !json.contains("pressedOptColor") ||
        !json.contains("highlightOptColor") || !json.contains("optionRender") ||
        !json.contains("triggerActionId")) {
        throw std::runtime_error("Missing critical fields for UIDropdown deserialization");
    }

    dropdown.setId(boost::uuids::string_generator()(json["id"].get<std::string>()));

    UIDropdown::Style style;
    style.normalColor = {json["normalColor"][0], json["normalColor"][1], json["normalColor"][2],
                         json["normalColor"][3]};
    style.hoverColor = {json["hoverColor"][0], json["hoverColor"][1], json["hoverColor"][2],
                        json["hoverColor"][3]};
    style.pressedColor = {json["pressedColor"][0], json["pressedColor"][1], json["pressedColor"][2],
                          json["pressedColor"][3]};
    style.arrowColor = {json["arrowColor"][0], json["arrowColor"][1], json["arrowColor"][2],
                        json["arrowColor"][3]};
    style.textColor = {json["textColor"][0], json["textColor"][1], json["textColor"][2],
                       json["textColor"][3]};
    style.normalOptColor = {json["normalOptColor"][0], json["normalOptColor"][1],
                            json["normalOptColor"][2], json["normalOptColor"][3]};
    style.hoverOptColor = {json["hoverOptColor"][0], json["hoverOptColor"][1],
                           json["hoverOptColor"][2], json["hoverOptColor"][3]};
    style.pressedOptColor = {json["pressedOptColor"][0], json["pressedOptColor"][1],
                             json["pressedOptColor"][2], json["pressedOptColor"][3]};
    style.highlightOptColor = {json["highlightOptColor"][0], json["highlightOptColor"][1],
                               json["highlightOptColor"][2], json["highlightOptColor"][3]};

    UIDropdown::OptionRender render;
    render.height = json["optionRender"]["height"];
    render.textHAlign = json["optionRender"]["textHAlign"];
    render.textVAlign = json["optionRender"]["textVAlign"];
    render.mesh = assetManger->getPrimitiveMesh(PrimitiveMeshLibrary::PrimitiveMesh::Quad);
    render.font = assetManger->get<Font>(json["optionRender"]["font"].get<std::string>());
    std::string shaderPath = json["optionRender"]["materialPath"].get<std::string>();
    auto material = std::make_shared<dzemikk::Material>();
    material->setShader(assetManger->get<Shader>(shaderPath));
    render.material = material;

    std::vector<UIDropdown::Option> opts;
    for (const auto& optionJson : json["options"]) {
        opts.push_back({.text = optionJson["text"].get<std::string>(),
                        .value = optionJson["value"].get<std::string>()});
    }

    const std::size_t selectedIndex =
        json.contains("selectedIndex")
            ? json["selectedIndex"].get<std::size_t>()
            : UIDropdown::noSelection();

    dropdown.init(style, render, opts, selectedIndex,
                  boost::uuids::string_generator()(json["triggerActionId"].get<std::string>()));
}

inline void postUIDropdownDeserialize(Component& component) {
    auto* dropdown = dynamic_cast<UIDropdown*>(&component);
    if (dropdown == nullptr) {
        throw std::runtime_error("Component type mismatch for UIDropdown post-deserialization");
    }
    dropdown->applyVisualState();
    dropdown->updateOptionVisuals();
    if (auto* optionsContainer = dropdown->getOptionsContainerGO(); optionsContainer != nullptr) {
        optionsContainer->enabled(false);
    }
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
            from_json(context.json, *dropdown, context.assetManager);
        },
        postUIDropdownDeserialize);
}
} // namespace dzemikk
#endif
