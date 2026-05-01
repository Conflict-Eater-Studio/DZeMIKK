#ifndef DZEMIKK_MESHRENDERERSERIALIZER_H
#define DZEMIKK_MESHRENDERERSERIALIZER_H

#include "ecs/components/meshRenderer.h"
#include "nlohmann/json.hpp"

namespace dzemikk {
inline void to_json(nlohmann::json& json, const MeshRenderer& meshRenderer) {
    json["type"] = meshRenderer.typeName();
    json["id"] = boost::uuids::to_string(meshRenderer.getId());
    json["model"] = meshRenderer.getModel();
}
void to_json(nlohmann::json& j, const dzemikk::Model::SubMesh& sm) {
    json = {
        {"mesh", sm.mesh ? sm.mesh->name : ""},
        {"materialIndex", sm.materialIndex}
    };
}

void to_json(nlohmann::json& json, const dzemikk::Model& model) {
    json = {
        {"subMeshes", model.getSubMeshes()}
    };
}
inline void from_json(const nlohmann::json& json, MeshRenderer& slider) {
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
        [](GameObject& gameObject, const nlohmann::json& componentJson) {
            auto* slider = gameObject.addComponent<UISlider>();
            from_json(componentJson, *slider);
        });
}
}
#endif
