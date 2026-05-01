#ifndef DZEMIKK_COMPONENTSERIALIZERREGISTRY_H
#define DZEMIKK_COMPONENTSERIALIZERREGISTRY_H

#pragma once

#include "ecs/component.h"
#include "ecs/components/monoBehaviour.h"
#include "ecs/gameobject.h"

#include <concepts>
#include <functional>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

namespace dzemikk {
class GameObject;

class ComponentSerializerRegistry {
  public:
    using SerializeFn = std::function<nlohmann::json(const Component&)>;
    using DeserializeIntoGameObjectFn = std::function<void(GameObject&, const nlohmann::json&)>;

    struct Entry {
        SerializeFn serialize;
        DeserializeIntoGameObjectFn deserializeIntoGameObject;
    };

    static ComponentSerializerRegistry& get();

    void registerType(std::string typeName, SerializeFn serializeFn,
                      DeserializeIntoGameObjectFn deserializeFn);

    template <typename T>
        requires std::derived_from<T, MonoBehaviour>
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
            [](GameObject& gameObject, const nlohmann::json& componentJson) {
                auto* script = gameObject.addComponent<T>();
                from_json(componentJson, *script);
            });
    }

    [[nodiscard]] nlohmann::json serialize(const Component& component) const;
    void deserializeIntoGameObject(GameObject& gameObject, const nlohmann::json& json) const;

  private:
    std::unordered_map<std::string, Entry> _entries;
};
} // namespace dzemikk

#endif // DZEMIKK_COMPONENTSERIALIZERREGISTRY_H
