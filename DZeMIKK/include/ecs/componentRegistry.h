#ifndef DZEMIKK_COMPONENTREGISTRY_H
#define DZEMIKK_COMPONENTREGISTRY_H
#pragma once
#include "component.h"

#include <concepts>
#include <cstddef>
#include <ranges>
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
        registerRecursive<T>(component);
    }

    void unregisterComponent(Component* component) {
        if (!component) {
            return;
        }
        auto typeIter = _componentToTypes.find(component);
        if (typeIter == _componentToTypes.end()) {
            return;
        }

        for (const std::type_index& typeIdx : typeIter->second) {
            auto vecIter = _components.find(typeIdx);
            auto indexMapIter = _componentIndices.find(typeIdx);
            if (vecIter == _components.end() || indexMapIter == _componentIndices.end()) {
                continue;
            }

            auto& vec = vecIter->second;
            auto& indices = indexMapIter->second;

            auto componentIndexIter = indices.find(component);
            if (componentIndexIter == indices.end() || vec.empty()) {
                continue;
            }

            std::size_t removeIndex = componentIndexIter->second;
            std::size_t lastIndex = vec.size() - 1;

            if (removeIndex != lastIndex) {
                Component* lastComponent = vec[lastIndex];
                vec[removeIndex] = lastComponent;
                indices[lastComponent] = removeIndex;
            }

            vec.pop_back();
            indices.erase(componentIndexIter);

            if (vec.empty()) {
                _components.erase(vecIter);
                _componentIndices.erase(indexMapIter);
            }
        }

        _componentToTypes.erase(typeIter);
    }

    void clear() {
        _components.clear();
        _componentIndices.clear();
        _componentToTypes.clear();
    }

    template <typename T>
        requires std::derived_from<T, Component>
    void getComponents(std::vector<T*>& out) {
        out.clear();
        auto iter = _components.find(std::type_index(typeid(T)));
        if (iter == _components.end()) {
            return;
        }
        for (Component* component : iter->second) {
            out.push_back(static_cast<T*>(component));
        }
    }

    template <typename T>
        requires std::derived_from<T, Component>
    void getEnabledComponents(std::vector<T*>& out) {
        out.clear();
        auto iter = _components.find(std::type_index(typeid(T)));
        if (iter == _components.end()) {
            return;
        }

        auto enabled =
            iter->second | std::views::filter([](Component* c) { return c->isEnabled(); });
        for (Component* component : enabled) {
            out.push_back(static_cast<T*>(component));
        }
    }

  private:
    ComponentRegistry() = default;
    ~ComponentRegistry() = default;

    template <typename T>
        requires std::derived_from<T, Component>
    void registerRecursive(T* component) {
        std::type_index idx(typeid(T));
        auto& components = _components[idx];
        components.push_back(component);
        _componentIndices[idx][component] = components.size() - 1;
        _componentToTypes[component].push_back(idx);
        if constexpr (requires { typename T::Base; }) {
            static_assert(!std::is_same_v<T, typename T::Base>,
                          "Base type cannot be the same as derived type");
            registerRecursive<typename T::Base>(static_cast<typename T::Base*>(component));
        }
    }

    // Type -> Component list (active scene only)
    std::unordered_map<std::type_index, std::vector<Component*>> _components;
    // Type -> (component -> index in _components[type])
    std::unordered_map<std::type_index, std::unordered_map<Component*, std::size_t>>
        _componentIndices;
    // Component -> list of types it was registered under
    std::unordered_map<Component*, std::vector<std::type_index>> _componentToTypes;
};

} // namespace dzemikk
#endif // DZEMIKK_COMPONENTREGISTRY_H
