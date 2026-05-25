#ifndef DZEMIKK_GAMEOBJECT_H
#define DZEMIKK_GAMEOBJECT_H

#include "ecs/components/ui/rectTransform.h"

#include <stdexcept>
#include <vcruntime_typeinfo.h>

#pragma once
#include "component.h"
#include "componentRegistry.h"
#include "components/monoBehaviour.h"
#include "components/transform.h"

#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <type_traits>
#include <vector>

namespace dzemikk {
class Scene;
class Canvas;
class RectTransform;
class GameObject {
  public:
    GameObject();
    GameObject(const boost::uuids::uuid& uuid);
    GameObject(const GameObject& other) = delete;
    GameObject& operator=(const GameObject& other) = delete;
    GameObject(GameObject&& other) noexcept = delete;
    GameObject& operator=(GameObject&& other) noexcept = delete;
    ~GameObject();

    // ---
    Transform* transform();
    [[nodiscard]] const Transform* transform() const;

    RectTransform* rectTransform();
    [[nodiscard]] const RectTransform* rectTransform() const;

    // --- Component operations
    /*
     * @brief Gets the first component of type T attached to this GameObject. Returns null if no
     * such component exists.
     */
    template <typename T> T* getComponent() {
        for (const auto& component : _components) {
            T* result = dynamic_cast<T*>(component.get());
            if (result) {
                return result;
            }
        }
        return nullptr;
    }

    template <typename T> const T* getComponent() const {
        for (const auto& component : _components) {
            const T* result = dynamic_cast<const T*>(component.get());
            if (result) {
                return result;
            }
        }
        return nullptr;
    }

    /*
     * @brief Gets all components of type T attached to this GameObject. Returns an empty vector if
     * no such components exist.
     */
    template <typename T> std::vector<T*> getComponents() const {
        std::vector<T*> results;
        for (const auto& component : _components) {
            T* result = dynamic_cast<T*>(component.get());
            if (result) {
                results.push_back(result);
            }
        }
        return results;
    }

    /*
     * @brief Adds a component of type T to this GameObject.
     * @param args Arguments to forward to the component's constructor.
     * @return A pointer to the added component.
     */
    template <typename T, typename... Args> T* addComponent(Args&&... args) {
        if (std::is_same_v<T, Transform> && _transform != nullptr) {
#if DZEMIKK_DEV_TOOLS
            spdlog::error("[{}] GameObject '{}' already has a Transform component. "
                          "GameObject::addComponent<Transform>() was called.",
                          boost::uuids::to_string(_id), _name);
#endif
            throw std::runtime_error("Transform component cannot be added manually");
        }

        if (std::is_same_v<T, RectTransform> && _rectTransform != nullptr) {
#if DZEMIKK_DEV_TOOLS
            spdlog::error("[{}] GameObject '{}' already has a RectTransform component. "
                          "GameObject::addComponent<RectTransform>() was called.",
                          boost::uuids::to_string(_id), _name);
#endif
            throw std::runtime_error("RectTransform component cannot be added manually");
        }

        // When we add a Canvas, make this gameobject have a RectTransform
        if (std::is_same_v<T, Canvas>) {
            replaceTransformWithRectTransform();
        }

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* result = component.get();
        if constexpr (std::is_base_of_v<MonoBehaviour, T>) {
            // For monobehaviours, set owner to this GameObject and cache the pointer for quick
            // access
            _monoBehaviours.push_back(result);
            if (_scene) {
                addScenePending(result);
            }
        }
        result->setOwner(this);
        ComponentRegistry::get().registerComponent<T>(result);
        component->enabled(_isEnabled);
        _components.push_back(std::move(component));
        return result;
    }

    /*
     * @brief Removes a component from this GameObject. The component will be destroyed and removed
     * from the scene if it is a MonoBehaviour.
     */
    template <typename T> void removeComponent(T* component) {
        if (!component) {
            return;
        }

        auto iter = std::ranges::find_if(_components.begin(), _components.end(),
                                         [component](const std::unique_ptr<Component>& comp) {
                                             return comp.get() == component;
                                         });
        if (iter != _components.end()) {
            if constexpr (std::is_base_of_v<MonoBehaviour, T>) {
                // Remove from monobehaviour cache if necessary
                auto monoIter = std::ranges::find(_monoBehaviours, component);
                if (monoIter != _monoBehaviours.end()) {
                    removeSceneActive(*monoIter);
                    _monoBehaviours.erase(monoIter);
                }
            }
            ComponentRegistry::get().unregisterComponent(component);
            _components.erase(iter);
        }
    }

    RectTransform* replaceTransformWithRectTransform();

    // -- Getters
    [[nodiscard]] GameObject* getParent() const;
    [[nodiscard]] const std::vector<GameObject*>& getChildren() const;
    [[nodiscard]] std::string getName() const;
    [[nodiscard]] boost::uuids::uuid getId() const;
    [[nodiscard]] const std::vector<MonoBehaviour*>& getMonoBehaviours() const;
    [[nodiscard]] const std::vector<std::unique_ptr<Component>>& getAllComponents() const;
    [[nodiscard]] bool hasStarted() const;
    [[nodiscard]] Scene* getScene();
    [[nodiscard]] bool isEnabled() const;

    // --- Setters
    void setId(const boost::uuids::uuid& uuid);
    void setName(const std::string& name);
    void setScene(Scene* scene);
    void markStarted();

    // --- Hierarchy operations
    /*
     * @brief Sets the parent of this GameObject. If the GameObject already has a parent, it will be
     * removed from the old parent's children. If the new parent is not null, this GameObject will
     * be added to the new parent's children.
     * @param parent The new parent GameObject. Can be null to detach from current parent.
     */
    void setParent(GameObject* parent);
    /*
     * @brief Adds a child GameObject to this GameObject. This is equivalent to calling
     * child->setParent(this).
     * @param child The child GameObject to add. Must not be null or this Game Object itself.
     */
    void addChild(GameObject* child);
    /**
     * @brief Creates a new child GameObject with the specified name and adds it to this GameObject.
     * @param name The name of the new child GameObject.
     */
    [[deprecated("Deprecated")]] void removeChild(GameObject* child);
    /*
     * @brief Detaches a child GameObject from this GameObject. The child will no longer have a
     * parent but will not be destroyed.
     * @param child The child GameObject to detach. Must be a current child of this GameObject.
     */
    void detachChild(GameObject* child);
    /*
     * @brief Detaches all child GameObjects from this GameObject. The children will no longer have
     * a parent but will not be destroyed.
     */
    void detachChildren();
    /*
     * @brief Destroys a child GameObject of this GameObject along with it's children.
     * The destroy is deffered until the end of the current update loop (update or fixedUpdate,
     * whichever runs first)
     */
    void destroyChild(GameObject* child);
    /*
     * @brief Destroys all child GameObjects of this GameObject along with their children.
     * The destroy is deffered until the end of the current update loop (update or fixedUpdate,
     * whichever runs first)
     */
    void destroyChildren();
    /**
     * @brief Sets all components of this GameObject (and it's children) to enabled or disabled.
     * @param isEnabled Whether to enable or disable the components.
     */
    void enabled(bool isEnabled);

    // --- Utility
    void addScenePending(MonoBehaviour* mono);
    void removeSceneActive(MonoBehaviour* mono);

  private:
    boost::uuids::uuid _id;
    std::string _name;
    bool _hasStarted = false;
    GameObject* _parent = nullptr;
    std::vector<GameObject*> _children;
    bool _isEnabled = true;

    Scene* _scene = nullptr;

    Transform* _transform = nullptr;
    RectTransform* _rectTransform = nullptr;

    std::vector<std::unique_ptr<Component>> _components;
    std::vector<MonoBehaviour*> _monoBehaviours;
};
} // namespace dzemikk

#endif // DZEMIKK_GAMEOBJECT_H
