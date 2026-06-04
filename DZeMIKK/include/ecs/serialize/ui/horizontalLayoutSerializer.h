#ifndef DZEMIKK_HORIZONTALLAYOUTSERIALIZER_H
#define DZEMIKK_HORIZONTALLAYOUTSERIALIZER_H
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/components/ui/horizontalLayout.h"
#include <nlohmann/json.hpp>

namespace dzemikk {

inline void to_json(nlohmann::json& j, const HorizontalLayout& layout) {
    j["type"] = layout.typeName();
    j["id"] = boost::uuids::to_string(layout.getId());
    j["spacing"] = layout.getSpacing();
    j["childForceExpandWidth"] = layout.getChildForceExpandWidth();
    j["childForceExpandHeight"] = layout.getChildForceExpandHeight();
}

inline void from_json(const nlohmann::json& json, HorizontalLayout& layout) {
    if (!json.contains("type") || !json["type"].is_string() || json["type"] != layout.typeName()) {
        throw std::runtime_error("Invalid component type for HorizontalLayout deserialization");
    }

    if (!json.contains("id") || !json.contains("spacing") ||
        !json.contains("childForceExpandWidth") || !json.contains("childForceExpandHeight")) {
        throw std::runtime_error("Missing fields for HorizontalLayout deserialization");
    }

    layout.setId(boost::uuids::string_generator()(json["id"].get<std::string>()));

    layout.setSpacing(json["spacing"].get<float>());
    layout.setChildForceExpandWidth(json["childForceExpandWidth"].get<bool>());
    layout.setChildForceExpandHeight(json["childForceExpandHeight"].get<bool>());

    layout.rebuild();
}

inline void registerHorizontalLayoutSerializer(ComponentSerializerRegistry& registry) {
    registry.registerType(
        "HorizontalLayout",
        [](const Component& component) {
            const auto* layout = dynamic_cast<const HorizontalLayout*>(&component);
            if (!layout) throw std::runtime_error("Type mismatch in HorizontalLayout serialization");

            nlohmann::json j;
            to_json(j, *layout);
            return j;
        },
        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto* layout = context.gameObject.addComponent<HorizontalLayout>();
            from_json(context.json, *layout);
        });
}

} // namespace dzemikk
#endif
