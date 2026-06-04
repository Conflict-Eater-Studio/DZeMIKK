#ifndef DZEMIKK_VERTICALLAYOUTSERIALIZER_H
#define DZEMIKK_VERTICALLAYOUTSERIALIZER_H
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/components/ui/verticalLayout.h"
#include <nlohmann/json.hpp>

namespace dzemikk {

inline void to_json(nlohmann::json& j, const VerticalLayout& layout) {
    j["type"] = layout.typeName();
    j["id"] = boost::uuids::to_string(layout.getId());
    j["spacing"] = layout.getSpacing();
    j["childForceExpandWidth"] = layout.getChildForceExpandWidth();
    j["childForceExpandHeight"] = layout.getChildForceExpandHeight();
}

inline void from_json(const nlohmann::json& json, VerticalLayout& layout) {
    if (!json.contains("type") || !json["type"].is_string() || json["type"] != layout.typeName()) {
        throw std::runtime_error("Invalid component type for VerticalLayout deserialization");
    }

    if (!json.contains("id") || !json.contains("spacing") ||
        !json.contains("childForceExpandWidth") || !json.contains("childForceExpandHeight")) {
        throw std::runtime_error("Missing fields for VerticalLayout deserialization");
    }

    layout.setId(boost::uuids::string_generator()(json["id"].get<std::string>()));

    layout.setSpacing(json["spacing"].get<float>());
    layout.setChildForceExpandWidth(json["childForceExpandWidth"].get<bool>());
    layout.setChildForceExpandHeight(json["childForceExpandHeight"].get<bool>());

    layout.rebuild();
}

inline void registerVerticalLayoutSerializer(ComponentSerializerRegistry& registry) {
    registry.registerType(
        "VerticalLayout",
        [](const Component& component) {
            const auto* layout = dynamic_cast<const VerticalLayout*>(&component);
            if (!layout) throw std::runtime_error("Type mismatch in VerticalLayout serialization");

            nlohmann::json j;
            to_json(j, *layout);
            return j;
        },
        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto* layout = context.gameObject.addComponent<VerticalLayout>();
            from_json(context.json, *layout);
        });
}

} // namespace dzemikk
#endif
