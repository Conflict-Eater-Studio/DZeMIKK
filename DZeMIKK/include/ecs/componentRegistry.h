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

class Scene;

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

    void setActiveScene(Scene* scene) {
        _activeScene = scene;
    }

    void setComponentScene(Component* component, Scene* scene) {
        auto infoIt = _componentInfo.find(component);
        if (infoIt == _componentInfo.end()) {
            return;
        }

        Scene* oldScene = infoIt->second.first;
        if (oldScene == scene) {
            return;
        }

        const std::vector<std::type_index>& types = infoIt->second.second;

        for (const std::type_index& typeIdx : types) {
            auto oldSceneIt = _sceneComponents.find(oldScene);
            if (oldSceneIt == _sceneComponents.end()) {
                continue;
            }
            auto oldTypeIt = oldSceneIt->second.find(typeIdx);
            if (oldTypeIt == oldSceneIt->second.end()) {
                continue;
            }

            auto& oldVec = oldTypeIt->second;
            auto oldIndexIt = _sceneComponentIndices[oldScene][typeIdx].find(component);
            if (oldIndexIt == _sceneComponentIndices[oldScene][typeIdx].end()) {
                continue;
            }

            std::size_t removeIdx = oldIndexIt->second;
            std::size_t lastIdx = oldVec.size() - 1;

            if (removeIdx != lastIdx) {
                Component* lastComp = oldVec[lastIdx];
                oldVec[removeIdx] = lastComp;
                _sceneComponentIndices[oldScene][typeIdx][lastComp] = removeIdx;
            }
            oldVec.pop_back();
            _sceneComponentIndices[oldScene][typeIdx].erase(oldIndexIt);

            if (oldVec.empty()) {
                oldSceneIt->second.erase(oldTypeIt);
                _sceneComponentIndices[oldScene].erase(typeIdx);
                if (oldSceneIt->second.empty()) {
                    _sceneComponents.erase(oldSceneIt);
                }
            }

            _sceneComponents[scene][typeIdx].push_back(component);
            _sceneComponentIndices[scene][typeIdx][component] =
                _sceneComponents[scene][typeIdx].size() - 1;
        }

        infoIt->second.first = scene;
    }

    template <typename T>
        requires std::derived_from<T, Component>
    void registerComponent(T* component) {
        if (!component || _componentInfo.contains(component)) {
            return;
        }
        registerRecursive<T>(component);
    }

    void unregisterComponent(Component* component) {
        if (!component) {
            return;
        }
        auto infoIt = _componentInfo.find(component);
        if (infoIt == _componentInfo.end()) {
            return;
        }

        Scene* scene = infoIt->second.first;
        const std::vector<std::type_index>& types = infoIt->second.second;

        for (const std::type_index& typeIdx : types) {
            auto sceneCompIt = _sceneComponents.find(scene);
            if (sceneCompIt == _sceneComponents.end()) {
                continue;
            }

            auto typeIt = sceneCompIt->second.find(typeIdx);
            if (typeIt == sceneCompIt->second.end()) {
                continue;
            }

            auto& indices = _sceneComponentIndices[scene][typeIdx];
            auto& vec = typeIt->second;

            auto indexIt = indices.find(component);
            if (indexIt == indices.end() || vec.empty()) {
                continue;
            }

            std::size_t removeIndex = indexIt->second;
            std::size_t lastIndex = vec.size() - 1;

            if (removeIndex != lastIndex) {
                Component* lastComponent = vec[lastIndex];
                vec[removeIndex] = lastComponent;
                indices[lastComponent] = removeIndex;
            }

            vec.pop_back();
            indices.erase(indexIt);

            if (vec.empty()) {
                sceneCompIt->second.erase(typeIt);
                _sceneComponentIndices[scene].erase(typeIdx);
                if (sceneCompIt->second.empty()) {
                    _sceneComponents.erase(sceneCompIt);
                }
            }
        }

        _componentInfo.erase(infoIt);
    }

    void clear() {
        _sceneComponents.clear();
        _sceneComponentIndices.clear();
        _componentInfo.clear();
    }

    template <typename T>
        requires std::derived_from<T, Component>
    void getComponents(std::vector<T*>& out) {
        out.clear();
        auto typeIdx = std::type_index(typeid(T));
        if (!_activeScene) {
            for (const auto& [scene, typeMap] : _sceneComponents) {
                auto typeIt = typeMap.find(typeIdx);
                if (typeIt != typeMap.end()) {
                    for (Component* component : typeIt->second) {
                        out.push_back(static_cast<T*>(component));
                    }
                }
            }
            return;
        }
        auto sceneIt = _sceneComponents.find(_activeScene);
        if (sceneIt == _sceneComponents.end()) {
            return;
        }
        auto typeIt = sceneIt->second.find(typeIdx);
        if (typeIt == sceneIt->second.end()) {
            return;
        }
        for (Component* component : typeIt->second) {
            out.push_back(static_cast<T*>(component));
        }
    }

    template <typename T>
        requires std::derived_from<T, Component>
    void getEnabledComponents(std::vector<T*>& out) {
        out.clear();
        auto typeIdx = std::type_index(typeid(T));
        if (!_activeScene) {
            for (const auto& [scene, typeMap] : _sceneComponents) {
                auto typeIt = typeMap.find(typeIdx);
                if (typeIt != typeMap.end()) {
                    for (Component* component : typeIt->second) {
                        if (component->isEnabled()) {
                            out.push_back(static_cast<T*>(component));
                        }
                    }
                }
            }
            return;
        }
        auto sceneIt = _sceneComponents.find(_activeScene);
        if (sceneIt == _sceneComponents.end()) {
            return;
        }
        auto typeIt = sceneIt->second.find(typeIdx);
        if (typeIt == sceneIt->second.end()) {
            return;
        }
        for (Component* component : typeIt->second) {
            if (component->isEnabled()) {
                out.push_back(static_cast<T*>(component));
            }
        }
    }

  private:
    ComponentRegistry() = default;
    ~ComponentRegistry() = default;

    template <typename T>
        requires std::derived_from<T, Component>
    void registerRecursive(T* component) {
        auto infoIt = _componentInfo.find(component);
        Scene* scene = nullptr;
        if (infoIt != _componentInfo.end()) {
            scene = infoIt->second.first;
        }

        std::type_index idx(typeid(T));
        _sceneComponents[scene][idx].push_back(component);
        _sceneComponentIndices[scene][idx][component] = _sceneComponents[scene][idx].size() - 1;

        if (infoIt == _componentInfo.end()) {
            _componentInfo[component] = {scene, {idx}};
        } else {
            infoIt->second.second.push_back(idx);
        }

        if constexpr (requires { typename T::Base; }) {
            static_assert(!std::is_same_v<T, typename T::Base>,
                          "Base type cannot be the same as derived type");
            registerRecursive<typename T::Base>(static_cast<typename T::Base*>(component));
        }
    }

    // Scene -> (Type -> Component list)
    std::unordered_map<Scene*, std::unordered_map<std::type_index, std::vector<Component*>>>
        _sceneComponents;
    // Scene -> (Type -> (Component -> index))
    std::unordered_map<
        Scene*, std::unordered_map<std::type_index, std::unordered_map<Component*, std::size_t>>>
        _sceneComponentIndices;
    // Component -> (Scene, list of types it was registered under)
    std::unordered_map<Component*, std::pair<Scene*, std::vector<std::type_index>>> _componentInfo;
    // Currently active scene for filtering queries
    Scene* _activeScene = nullptr;
};

} // namespace dzemikk
#endif // DZEMIKK_COMPONENTREGISTRY_H
