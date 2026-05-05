#pragma once
#ifndef DZEMIKK_COMPONENTSERIALIZERREGISTRY_H
#define DZEMIKK_COMPONENTSERIALIZERREGISTRY_H


#include "assetManager/assetmanager.h"
#include "ecs/component.h"
#include "ecs/components/monoBehaviour.h"
#include "ecs/gameobject.h"

#include <concepts>
#include <functional>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace dzemikk {

class GameObject;
class ComponentSerializerRegistry {
  public:
    struct DeserializationContext {
        GameObject& gameObject;
        AssetManager* assetManager;
        const nlohmann::json& json;
    };

    using SerializeFn = std::function<nlohmann::json(const Component&)>;
    using DeserializeIntoGameObjectFn = std::function<void(DeserializationContext context)>;

    struct Entry {
        SerializeFn serialize;
        DeserializeIntoGameObjectFn deserializeIntoGameObject;
    };

    static ComponentSerializerRegistry& get();

    void registerType(std::string typeName, SerializeFn serializeFn, DeserializeIntoGameObjectFn deserializeFn);

    template <typename T> requires std::derived_from<T, MonoBehaviour>
    void registerType(std::string typeName) {
        std::string resolvedTypeName = std::move(typeName);

        registerType(
            resolvedTypeName,
            [resolvedTypeName](const Component& component) {
                const auto* script = dynamic_cast<const T*>(&component);
                if (script == nullptr) {
                    throw std::runtime_error("Component type mismatch for " + resolvedTypeName);
                }
                return nlohmann::json(*script);
            },
            [](const DeserializationContext& context) {
                auto* script = context.gameObject.addComponent<T>();
                from_json(context.json, context.gameObject.getComponent<T>());
            });
    }

    [[nodiscard]] nlohmann::json serialize(const Component& component) const;
    void deserializeIntoGameObject(const DeserializationContext& context) const;

  private:
    std::unordered_map<std::string, Entry> _entries;
};
}

#endif