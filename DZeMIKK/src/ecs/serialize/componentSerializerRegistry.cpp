#include "ecs/serialize/componentSerializerRegistry.h"

#include "ecs/serialize/rectTransformSerializer.h"
#include "ecs/serialize/transformSerializer.h"

#include <stdexcept>
#include <utility>

namespace dzemikk {
namespace {
ComponentSerializerRegistry buildDefaultRegistry() {
    ComponentSerializerRegistry registry;

    registerTransformSerializer(registry);
    registerRectTransformSerializer(registry);

    return registry;
}
} // namespace

ComponentSerializerRegistry& ComponentSerializerRegistry::get() {
    static ComponentSerializerRegistry kRegistry = buildDefaultRegistry();
    return kRegistry;
}

void ComponentSerializerRegistry::registerType(std::string typeName, SerializeFn serializeFn,
                                               DeserializeIntoGameObjectFn deserializeFn) {
    _entries[std::move(typeName)] = Entry{.serialize = std::move(serializeFn),
                                          .deserializeIntoGameObject = std::move(deserializeFn)};
}

nlohmann::json ComponentSerializerRegistry::serialize(const Component& component) const {
    const auto iter = _entries.find(component.typeName());
    if (iter == _entries.end()) {
        throw std::runtime_error("No serializer registered for component type: " +
                                 component.typeName());
    }

    return iter->second.serialize(component);
}

void ComponentSerializerRegistry::deserializeIntoGameObject(GameObject& gameObject,
                                                            const nlohmann::json& json) const {
    if (!json.contains("type") || !json["type"].is_string()) {
        throw std::runtime_error("Serialized component is missing string field 'type'");
    }

    const std::string typeName = json["type"].get<std::string>();
    const auto iter = _entries.find(typeName);
    if (iter == _entries.end()) {
        throw std::runtime_error("No deserializer registered for component type: " + typeName);
    }

    iter->second.deserializeIntoGameObject(gameObject, json);
}
} // namespace dzemikk