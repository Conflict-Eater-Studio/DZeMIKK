#ifndef DZEMIKK_SCENE_H
#define DZEMIKK_SCENE_H

#include "ecs/components/monoBehaviour.h"

#include <boost/uuid/uuid.hpp>
#include <memory>
#include <unordered_set>
#include <vector>

namespace dzemikk {
class GameObject;
class MonoBehaviour;
class Octree;

class Scene {
  public:
    Scene();
    Scene(const Scene& other) = delete;
    Scene& operator=(const Scene& other) = delete;
    Scene(Scene&& other) noexcept = delete;
    Scene& operator=(Scene&& other) noexcept = delete;
    ~Scene();

    /*
     * @brief Creates a new GameObject in this scene and returns a pointer to it.
     */
    GameObject* createGameObject();

    /**
     * @brief Creates a new GameObject in this scene with a specific name and returns a pointer to
     * it.
     * @param name The name for the new GameObject.
     * @return A pointer to the newly created GameObject.
     */
    GameObject* createGameObject(const std::string& name);

    /**
     * @brief Creates a new GameObject in this scene with a specific name and parent, and returns a
     * pointer to it.
     * @param name The name for the new GameObject.
     * @param parent The parent GameObject for the new GameObject.
     * @return A pointer to the newly created GameObject.
     */
    GameObject* createGameObject(const std::string& name, GameObject* parent);

    /**
     * @brief Creates a new GameObject in this scene with a specific parent, and returns a pointer
     * to it.
     * @param parent The parent GameObject for the new GameObject.
     * @return A pointer to the newly created GameObject.
     */
    GameObject* createGameObject(GameObject* parent);

    /**
     * @brief Destroys a GameObject in this scene.
     * The destroy is deffered until the end of the current update loop (update or fixedUpdate,
     * whichever runs first)
     * @param object The GameObject to destroy. Must be a GameObject that was created by this scene
     * and not already pending destruction.
     */
    void destroyGameObject(GameObject* object);
    void update(double deltaTime);
    void fixedUpdate(double deltaTime);

    void processPendingStart();
    void addPending(MonoBehaviour* mono);
    void removeActive(MonoBehaviour* mono);
    void processDelete();
    void clearAllObjects();

    /**
     * @brief Finds the first GameObject in this scene with the specified ID. Returns null if no
     * such GameObject exists.
     *
     * @param id The ID of the GameObject to find.
     * @return A pointer to the found GameObject, or null if no such GameObject exists.
     */
    GameObject* findGameObjectById(const boost::uuids::uuid& id);
    /**
     * @brief Finds all GameObjects in this scene with the specified ID. Returns an empty vector if
     * no such GameObjects exist.
     *
     * @param id The ID of the GameObjects to find.
     * @return A vector of pointers to the found GameObjects, or an empty vector if no such
     * GameObjects exist.
     */
    std::vector<GameObject*> findGameObjectsById(const boost::uuids::uuid& id);
    /**
     * @brief Finds the first GameObject in this scene with the specified name. Returns null if no
     * such GameObject exists.
     *
     * @param name The name of the GameObject to find.
     * @return A pointer to the found GameObject, or null if no such GameObject exists.
     */
    GameObject* findGameObjectByName(const std::string& name);
    /**
     * @brief Finds all GameObjects in this scene with the specified name. Returns an empty vector
     * if no such GameObjects exist.
     *
     * @param name The name of the GameObjects to find.
     * @return A vector of pointers to the found GameObjects, or an empty vector if no such
     * GameObjects exist.
     */
    std::vector<GameObject*> findGameObjectsByName(const std::string& name);
    /**
     * @brief Finds the first GameObject in this scene with the specified tag. Returns null if no
     * such GameObject exists.
     *
     * @param tag The tag of the GameObject to find.
     * @return A pointer to the found GameObject, or null if no such GameObject exists.
     */
    GameObject* findGameObjectByTag(const std::string& tag);
    /**
     * @brief Finds all GameObjects in this scene with the specified tag. Returns an empty vector if
     * no such GameObjects exist.
     *
     * @param tag The tag of the GameObjects to find.
     * @return A vector of pointers to the found GameObjects, or an empty vector if no such
     * GameObjects exist.
     */
    std::vector<GameObject*> findGameObjectsByTag(const std::string& tag);

    void rebuildOctree();
    Octree* getOctree() const;

    [[nodiscard]] boost::uuids::uuid getId() const;
    [[nodiscard]] const std::vector<std::unique_ptr<dzemikk::GameObject>>& getObjects() const;
    void setId(const boost::uuids::uuid& uuid);

    /**
     * Register a GameObject with a specific name. Should not be called directly, use
     * GameObject::setName instead.
     *
     * @param name The name to register the GameObject under.
     * @param object The GameObject to register. Must be a GameObject that was created by this
     * scene.
     */
    void registerNamedObject(const std::string& name, GameObject* object);
    /**
     * Unregister a GameObject from a specific name. Should not be called directly, use
     * GameObject::setName instead.
     *
     * @param name The name to unregister the GameObject from.
     * @param object The GameObject to unregister. Must be a GameObject that was created by this
     * scene and currently registered under the specified name.
     */
    void unregisterNamedObject(const std::string& name, GameObject* object);

    /**
     * Register a GameObject with a specific tag. Should not be called directly, use
     * GameObject::addTag instead.
     *
     * @param tag The tag to register the GameObject under.
     * @param object The GameObject to register. Must be a GameObject that was created by this
     * scene.
     */
    void registerTaggedObject(const std::string& tag, GameObject* object);

    /**
     * Unregister a GameObject from a specific tag. Should not be called directly, use
     * GameObject::removeTag instead.
     *
     * @param tag The tag to unregister the GameObject from.
     * @param object The GameObject to unregister. Must be a GameObject that was created by this
     * scene and currently registered under the specified tag.
     */
    void unregisterTaggedObject(const std::string& tag, GameObject* object);

    /**
     * Register a GameObject with a specific ID. Should not be called directly, use
     * GameObject::setId instead.
     *
     * @param id The ID to register the GameObject under.
     * @param object The GameObject to register. Must be a GameObject that was created by this
     * scene.
     */
    void registerIdObject(const boost::uuids::uuid& id, GameObject* object);

    /**
     * Unregister a GameObject from a specific ID. Should not be called directly, use
     * GameObject::setId instead.
     *
     * @param id The ID to unregister the GameObject from.
     * @param object The GameObject to unregister. Must be a GameObject that was created by this
     * scene and currently registered under the specified ID.
     */
    void unregisterIdObject(const boost::uuids::uuid& id, GameObject* object);

  private:
    boost::uuids::uuid _id;
    std::vector<std::unique_ptr<dzemikk::GameObject>> _objects;
    std::vector<MonoBehaviour*> _pendingStart;
    std::vector<MonoBehaviour*> _active;
    std::unordered_set<MonoBehaviour*> _activeSet;
    std::vector<GameObject*> _pendingDestroy;
    std::unordered_map<std::string, std::vector<GameObject*>> _taggedObjects;
    std::unordered_map<std::string, std::vector<GameObject*>> _namedObjects;
    std::unordered_map<boost::uuids::uuid, std::vector<GameObject*>> _idObjects;

    std::unique_ptr<Octree> _octree;
};
} // namespace dzemikk

#endif // DZEMIKK_SCENE_H
