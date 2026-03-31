#include "../../include/core/scene.h"

#include "ecs/components/monobehaviour.h"
#include "ecs/gameobject.h"

#include <type_traits>

namespace dzemikk {
    GameObject* Scene::createGameObject() {
        auto object = std::make_unique<GameObject>();
        GameObject* result = object.get();
        _objects.push_back(std::move(object));
        return result;
    }

    void Scene::destroyGameObject(GameObject* object) {
        // Runs lieanr search
        // Unless we delete a ton of objects each frame, this *should* be fine
        if (!object) { return; }

        // Call onDestroy on all monobehaviours
        auto* mono = object->getComponent<MonoBehaviour>();
        if (mono) {
            mono->onDestroy();
        }

        // Copy for safety
        auto children = object->getChildren();

        // Delete children first
        for (const auto& child : children) {
            destroyGameObject(child);
        }

        // Detach from parent
        if(object->getParent()) {
            object->getParent()->removeChild(object);
        }

        // Erase from scene
        auto iter = std::ranges::find_if(_objects.begin(), _objects.end(),
            [object](const std::unique_ptr<GameObject>& obj) { return obj.get() == object; }
        );
        if (iter != _objects.end()) {
            _objects.erase(iter);
        }
    }

    void Scene::update(double deltaTime) {
        // Update all behaviours
        for (const auto& object : _objects) {
            for (const auto& mono : object->getMonoBehaviours()) {
                if (!mono->hasStarted()) {
                    mono->start();
                    mono->markStarted();
                }
                mono->update(deltaTime);
            }
        }

        // Late Update all behaviours
        for (const auto& object : _objects) {
            for (const auto& behaviour : object->getMonoBehaviours()) {
                behaviour->lateUpdate();
            }
        }
    }

    void Scene::fixedUpdate(double deltaTime) {
        // Fixed update also starts components that haven't started since it runs at a fixed interval
        for (const auto& object : _objects) {
            for (const auto& mono : object->getMonoBehaviours()) {
                if (!mono->hasStarted()) {
                    mono->start();
                    mono->markStarted();
                }
                mono->fixedUpdate(deltaTime);
            }
        }
    }
}
