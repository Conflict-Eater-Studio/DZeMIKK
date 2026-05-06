#ifndef DZEMIKK_GRIDLAYOUTSERIALIZER_H
#define DZEMIKK_GRIDLAYOUTSERIALIZER_H
#include "ecs/serialize/componentSerializerRegistry.h"
#include "ecs/components/ui/gridLayout.h"
#include <nlohmann/json.hpp>

namespace dzemikk {

inline void to_json(nlohmann::json& j, const GridLayout& layout) {
    j["type"] = layout.typeName();
    j["id"] = boost::uuids::to_string(layout.getId());
    j["cellSize"] = { layout.getCellSize().x, layout.getCellSize().y };
    j["spacing"] = { layout.getSpacing().x, layout.getSpacing().y };
    j["columns"] = layout.getColumns();
    j["startCorner"] = layout.getStartCorner();
}

inline void from_json(const nlohmann::json& json, GridLayout& layout) {
    if (!json.contains("type") || !json["type"].is_string() || json["type"] != layout.typeName()) {
        throw std::runtime_error("Invalid component type for GridLayout deserialization");
    }

    if (!json.contains("id") || !json.contains("cellSize") ||
        !json.contains("spacing") || !json.contains("columns")) {
        throw std::runtime_error("Missing fields for GridLayout deserialization");
    }

    layout.setId(boost::uuids::string_generator()(json["id"].get<std::string>()));

    layout.setCellSize({
        json["cellSize"][0].get<float>(),
        json["cellSize"][1].get<float>()
    });

    layout.setSpacing({
        json["spacing"][0].get<float>(),
        json["spacing"][1].get<float>()
    });

    layout.setColumns(json["columns"].get<int>());

    if (json.contains("startCorner")) {
        layout.setStartCorner(static_cast<LayoutStartCorner>(json["startCorner"].get<int>()));
    }

    layout.rebuild();
}

inline void registerGridLayoutSerializer(ComponentSerializerRegistry& registry) {
    registry.registerType(
        "GridLayout",
        [](const Component& component) {
            const auto* layout = dynamic_cast<const GridLayout*>(&component);
            if (!layout) throw std::runtime_error("Type mismatch in GridLayout serialization");

            nlohmann::json j;
            to_json(j, *layout);
            return j;
        },
        [](const ComponentSerializerRegistry::DeserializationContext& context) {
            auto* layout = context.gameObject.addComponent<GridLayout>();
            from_json(context.json, *layout);
        });
    }
}
#endif
