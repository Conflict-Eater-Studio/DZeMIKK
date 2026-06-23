#ifndef DZEMIKK_GAMEOBJECT_H
#define DZEMIKK_GAMEOBJECT_H

#include "ecs/components/ui/rectTransform.h"

#include <stdexcept>
#include <unordered_set>
// #include <vcruntime_typeinfo.h>

#pragma once
#include "component.h"
#include "componentRegistry.h"
#include "components/monoBehaviour.h"
#include "components/transform.h"

#include <memory>
#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif
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
        ComponentRegistry::get().setComponentScene(result, _scene);
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
    /**
     * @brief Sets the unique identifier of this GameObject. If this is called after the GameObject
     * is created, the GO is reregistered with new ID.
     * @param uuid The new unique identifier for this GameObject. Must be unique across the entire
     * scene.
     */
    void setId(const boost::uuids::uuid& uuid);
    /**
     * @brief Sets the name of this GameObject. Names are not required to be unique. If this
     * GameObject is part of a scene, the scene's name index will be updated accordingly.
     *
     * @param name The new name for this GameObject.
     */
    void setName(const std::string& name);
    /**
     * @brief Sets the scene of this GameObject. This should only be called by the Scene that
     * creates this GameObject. Setting the scene will register this GameObject with the scene's
     * internal indices and add any MonoBehaviour components to the scene's pending start list.
     * Once a scene is set it cannot be changed, and the GameObject cannot be part of multiple
     * scenes.
     *
     * @param scene The Scene to set for this GameObject. Must not be null and must not be a
     * different scene if this GameObject is already part of a scene.
     */
    void setScene(Scene* scene);
    /**
     * @brief Marks this GameObject as having started. This should only be called by the Scene when
     * processing pending starts. Marking a GameObject as started will move all its MonoBehaviour
     * components from the scene's pending start list to the active list, allowing them to be
     * updated in subsequent update loops.
     */
    void markStarted();

    // --- Child search
    /**
     * @brief Finds the first direct child GameObject with the specified name. Returns null if no
     * such child exists.
     *
     * @param name The name of the child GameObject to find.
     * @return A pointer to the found child GameObject, or null if no such child exists.
     */
    [[nodiscard]] GameObject* findChildByName(const std::string& name);
    /**
     * @brief Finds all direct child GameObjects with the specified name. Returns an empty vector if
     * no such children exist.
     *
     * @param name The name of the child GameObjects to find.
     * @return A vector of pointers to the found child GameObjects, or an empty vector if no such
     * children exist.
     */
    [[nodiscard]] std::vector<GameObject*> findChildrenByName(const std::string& name);
    /**
     * @brief Finds the first direct child GameObject with the specified tag. Returns null if no
     * such child exists.
     *
     * @param tag The tag of the child GameObject to find.
     * @return A pointer to the found child GameObject, or null if no such child exists.
     */
    [[nodiscard]] GameObject* findChildByTag(const std::string& tag);
    /**
     * @brief Finds all direct child GameObjects with the specified tag. Returns an empty vector if
     * no such children exist.
     *
     * @param tag The tag of the child GameObjects to find.
     * @return A vector of pointers to the found child GameObjects, or an empty vector if no such
     * children exist.
     */
    [[nodiscard]] std::vector<GameObject*> findChildrenByTag(const std::string& tag);

    // --- Deep child search
    /**
     * @brief Finds the first descendant GameObject (child, grandchild, etc.) with the specified
     * name. Returns null if no such descendant exists.
     *
     * @param name The name of the descendant GameObject to find.
     * @return A pointer to the found descendant GameObject, or null if no such descendant exists.
     */
    [[nodiscard]] GameObject* findDescendantByName(const std::string& name) const;
    /**
     * @brief Finds all descendant GameObjects (children, grandchildren, etc.) with the specified
     * name. Returns an empty vector if no such descendants exist.
     *
     * @param name The name of the descendant GameObjects to find.
     * @return A vector of pointers to the found descendant GameObjects, or an empty vector if no
     * such descendants exist.
     */
    [[nodiscard]] std::vector<GameObject*> findDescendantsByName(const std::string& name) const;
    /**
     * @brief Finds the first descendant GameObject (child, grandchild, etc.) with the specified
     * tag. Returns null if no such descendant exists.
     *
     * @param tag The tag of the descendant GameObject to find.
     * @return A pointer to the found descendant GameObject, or null if no such descendant exists.
     */
    [[nodiscard]] GameObject* findDescendantByTag(const std::string& tag) const;
    /**
     * @brief Finds all descendant GameObjects (children, grandchildren, etc.) with the specified
     * tag. Returns an empty vector if no such descendants exist.
     *
     * @param tag The tag of the descendant GameObjects to find.
     * @return A vector of pointers to the found descendant GameObjects, or an empty vector if no
     * such descendants exist.
     */
    [[nodiscard]] std::vector<GameObject*> findDescendantsByTag(const std::string& tag) const;

    // --- Tag operations
    /**
     * @brief Adds a tag to this GameObject. Tags are used for grouping and searching GameObjects.
     * A GameObject can have multiple tags. Adding the same tag multiple times has no effect.
     *
     * @param tag The tag to add.
     */
    void addTag(const std::string& tag);
    /**
     * @brief Removes a tag from this GameObject. If the GameObject does not have the specified tag,
     * this method does nothing.
     *
     * @param tag The tag to remove.
     */
    void removeTag(const std::string& tag);
    /**
     * @brief Checks if this GameObject has a specific tag.
     *
     * @param tag The tag to check for.
     * @return true if the GameObject has the specified tag, false otherwise.
     */
    [[nodiscard]] bool hasTag(const std::string& tag) const;
    /**
     * @brief Gets all tags currently assigned to this GameObject.
     *
     * @return A const reference to an unordered set containing all tags of this GameObject. The
     * returned set should not be modified directly; use addTag and removeTag to modify the tags.
     */
    [[nodiscard]] const std::unordered_set<std::string>& getTags() const;

    // --- Hierarchy operations
    /*
     * @brief Sets the parent of this GameObject. If the GameObject already has a parent, it will be
     * removed from the old parent's children. If the new parent is not null, this GameObject will
     * be added to the new parent's children.
     *
     * @param parent The new parent GameObject. Can be null to detach from current parent.
     *
     * @return true if the parent was successfully set, false if the operation was rejected (e.g.
     * due to cyclic parenting or invalid parent). In case of rejection, the parent will remain
     * unchanged.
     */
    bool setParent(GameObject* parent);
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
     * @brief Destroys this GameObject along with all its children. The destroy is deffered until
     * the end of the current update loop (update or fixedUpdate, whichever runs first)
     */
    void destroy();
    /**
     * @brief Sets all components of this GameObject (and it's children) to enabled or disabled.
     * @param isEnabled Whether to enable or disable the components.
     */
    void enabled(bool isEnabled);

    // --- Utility
    void addScenePending(MonoBehaviour* mono);
    void removeSceneActive(MonoBehaviour* mono);

    [[nodiscard]] uint32_t getLastRaycastQueryId() const {
        return _lastRaycastQueryId;
    }
    void setLastRaycastQueryId(uint32_t queryId) {
        _lastRaycastQueryId = queryId;
    }

  private:
    boost::uuids::uuid _id;
    std::string _name = "GameObject";
    std::unordered_set<std::string> _tags;
    bool _hasStarted = false;
    GameObject* _parent = nullptr;
    std::vector<GameObject*> _children;
    bool _isEnabled = true;

    Scene* _scene = nullptr;

    Transform* _transform = nullptr;
    RectTransform* _rectTransform = nullptr;

    std::vector<std::unique_ptr<Component>> _components;
    std::vector<MonoBehaviour*> _monoBehaviours;

    uint32_t _lastRaycastQueryId = 0;
};
} // namespace dzemikk

#endif // DZEMIKK_GAMEOBJECT_H
