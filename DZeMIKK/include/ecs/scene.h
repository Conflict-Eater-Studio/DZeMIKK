#ifndef DZEMIKK_SCENE_H
#define DZEMIKK_SCENE_H

#include "ecs/components/monoBehaviour.h"

#include <boost/uuid/uuid.hpp>
#include <memory>
#include <vector>

namespace dzemikk {
class GameObject;
class MonoBehaviour;
class Scene {
  public:
    Scene();
    Scene(const Scene& other) = delete;
    Scene& operator=(const Scene& other) = delete;
    Scene(Scene&& other) noexcept = delete;
    Scene& operator=(Scene&& other) noexcept = delete;
    ~Scene() = default;

    /*
     * @brief Creates a new GameObject in this scene and returns a pointer to it.
     */
    GameObject* createGameObject();
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

    [[nodiscard]] boost::uuids::uuid getId() const;
    [[nodiscard]] const std::vector<std::unique_ptr<dzemikk::GameObject>>& getObjects() const;
    void setId(const boost::uuids::uuid& uuid);

  private:
    boost::uuids::uuid _id;
    std::vector<std::unique_ptr<dzemikk::GameObject>> _objects;
    std::vector<MonoBehaviour*> _pendingStart;
    std::vector<MonoBehaviour*> _active;
    std::vector<GameObject*> _pendingDestroy;
};
} // namespace dzemikk

#endif // DZEMIKK_SCENE_H
