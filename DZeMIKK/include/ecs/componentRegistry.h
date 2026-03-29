#ifndef DZEMIKK_COMPONENTREGISTRY_H
#define DZEMIKK_COMPONENTREGISTRY_H

#pragma once

#include "component.h"
#include "gameobject.h"
#include "scene.h"

#include <concepts>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace dzemikk {
class ComponentRegistry {
  public:
    ComponentRegistry(const ComponentRegistry&) = delete;
    void operator=(const ComponentRegistry&) = delete;
    ComponentRegistry(ComponentRegistry&&) = delete;
    void operator=(ComponentRegistry&&) = delete;

    static ComponentRegistry& get() {
        static ComponentRegistry instance;
        return instance;
    }

    template <typename T>
        requires std::derived_from<T, Component>
    void registerComponent(T* component) {
        if (!component || _componentToTypes.contains(component)) {
            return;
        }

        Component* base = component;
        const GameObject* obj = base->getOwner();

        if (const Scene* scene = &obj->getScene(); scene) {
            registerRecursive<T>(component, scene);
        }
    }

    void unregisterComponent(Component* component) {
        if (!component) {
            return;
        }

        auto typeIter = _componentToTypes.find(component);
        if (typeIter == _componentToTypes.end()) {
            return;
        }

        Component* base = component;
        auto* owner = base->getOwner();

        if (!owner) {
            return;
        }

        const Scene* scene = &owner->getScene();
        auto sceneIter = _components.find(scene);

        if (sceneIter == _components.end()) {
            return;
        }

        for (const std::type_index& typeIdx : typeIter->second) {
            auto& vec = sceneIter->second[typeIdx];
            std::erase(vec, component);
        }

        if (sceneIter->second.empty()) {
            _components.erase(sceneIter);
        }

        _componentToTypes.erase(typeIter);
    }

    template <typename T>
        requires std::derived_from<T, Component>
    void getComponents(const Scene* scene, std::vector<T*>& out) {
        out.clear();

        if (!scene) {
            return;
        }

        auto sceneIter = _components.find(scene);
        if (sceneIter == _components.end()) {
            return;
        }

        auto typeIter = sceneIter->second.find(std::type_index(typeid(T)));
        if (typeIter == sceneIter->second.end()) {
            return;
        }

        out.reserve(typeIter->second.size());
        for (Component* component : typeIter->second) {
            out.push_back(static_cast<T*>(component));
        }
    }

    template <typename T>
        requires std::derived_from<T, Component>
    void getComponents(std::vector<T*>& out) {
        out.clear();

        for (const auto& [scene, typeComponents] : _components) {
            if (auto typeIter = typeComponents.find(std::type_index(typeid(T)));
                typeIter != typeComponents.end()) {
                for (Component* component : typeIter->second) {
                    out.push_back(static_cast<T*>(component));
                }
            }
        }
    }

  private:
    ComponentRegistry() = default;
    ~ComponentRegistry() = default;

    template <typename T>
        requires std::derived_from<T, Component>
    void registerRecursive(T* component, const Scene* scene) {
        std::type_index idx(typeid(T));
        _components[scene][idx].push_back(component);
        _componentToTypes[component].push_back(idx);

        if constexpr (requires { typename T::Base; }) {
            static_assert(!std::is_same_v<T, typename T::Base>,
                          "Base type cannot be the same as derived type");
            registerRecursive<typename T::Base>(static_cast<typename T::Base*>(component), scene);
        }
    }

    // Map: Scene -> (Type -> Component List)
    std::unordered_map<const Scene*, std::unordered_map<std::type_index, std::vector<Component*>>>
        _components;

    // Map: Component -> List of Types (under which it was registered)
    std::unordered_map<Component*, std::vector<std::type_index>> _componentToTypes;
};
} // namespace dzemikk

#endif // DZEMIKK_COMPONENTREGISTRY_H