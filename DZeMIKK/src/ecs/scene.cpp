#include "ecs/scene.h"

#include "ecs/components/monoBehaviour.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"

#include <algorithm>
#include <stack>
#include <unordered_set>

namespace dzemikk {
Scene::Scene() : _id(boost::uuids::random_generator()()) {};

GameObject* Scene::createGameObject() {
    auto object = std::make_unique<GameObject>();
    GameObject* result = object.get();
    result->setScene(this);
    _objects.push_back(std::move(object));
    return result;
}

void Scene::destroyGameObject(GameObject* object) {
    // Check if object is valid, not already pending destruction and belongs to this scene
    if (!object || std::ranges::find(_pendingDestroy, object) != _pendingDestroy.end() ||
        std::ranges::find_if(_objects, [object](const auto& obj) { return obj.get() == object; }) ==
            _objects.end()) {
        return;
    }

    std::vector<GameObject*> toDestroy;
    std::stack<GameObject*> stack;
    std::unordered_set<GameObject*> visited;
    stack.push(object);

    // Collect all children in DFS order
    while (!stack.empty()) {
        GameObject* current = stack.top();
        stack.pop();

        if (!current || !visited.insert(current).second) {
            continue;
        }

        toDestroy.push_back(current);
        for (const auto& child : current->getChildren()) {
            stack.push(child);
        }
    }

    // Destroy children first
    std::ranges::reverse(toDestroy);

    // Check for duplicates in _pendingDestroy to avoid double deletion
    // And append to _pendingDestroy if not already present
    std::unordered_set<GameObject*> pendingSet(_pendingDestroy.begin(), _pendingDestroy.end());
    for (GameObject* obj : toDestroy) {
        if (pendingSet.insert(obj).second) {
            _pendingDestroy.push_back(obj);
        }
    }
}

void Scene::update(double deltaTime) {
    // Update all behaviours
    processPendingStart();

    const auto active_snapshot = _active;
    for (auto* mono : active_snapshot) {
        if (!mono || std::ranges::find(_active, mono) == _active.end()) {
            continue;
        }
        mono->update(deltaTime);
    }

    // Late Update all behaviours
    for (auto* mono : active_snapshot) {
        if (!mono || std::ranges::find(_active, mono) == _active.end()) {
            continue;
        }
        mono->lateUpdate();
    }

    processDelete();
}

void Scene::fixedUpdate(double deltaTime) {
    // Fixed update also starts components that haven't started since it runs at a fixed interval
    processPendingStart();

    const auto active_snapshot = _active;
    for (auto* mono : active_snapshot) {
        if (!mono || std::ranges::find(_active, mono) == _active.end()) {
            continue;
        }
        mono->fixedUpdate(deltaTime);
    }

    processDelete();
}

void Scene::processPendingStart() {
    while (!_pendingStart.empty()) {
        std::vector<MonoBehaviour*> start;
        std::swap(start, _pendingStart);

        auto filtered =
            std::ranges::remove_if(start, [](MonoBehaviour* mono) { return !mono->hasStarted(); });
        for (const auto& mono : filtered) {
            mono->start();
            mono->markStarted();
            _active.push_back(mono);
        }
    }
}

void Scene::processDelete() {
    for (auto* obj : _pendingDestroy) {
        const auto& monos = obj->getMonoBehaviours();
        for (const auto& mono : monos) {
            mono->onDestroy();
        }
        auto inMonos = [&](MonoBehaviour* mono) {
            return std::ranges::find(monos, mono) != monos.end();
        };
        std::erase_if(_active, inMonos);
        std::erase_if(_pendingStart, inMonos);

        if (obj->getParent()) {
            obj->getParent()->detachChild(obj);
        }

        std::erase_if(_objects, [obj](const auto& lObj) { return lObj.get() == obj; });
    }

    _pendingDestroy.clear();
}

void Scene::addPending(MonoBehaviour* mono) {
    if (!mono || std::ranges::find(_pendingStart, mono) != _pendingStart.end() ||
        std::ranges::find(_active, mono) != _active.end()) {
        return;
    }
    _pendingStart.push_back(mono);
}

void Scene::removeActive(MonoBehaviour* mono) {
    std::erase(_active, mono);
    std::erase(_pendingStart, mono);
}

boost::uuids::uuid Scene::getId() const {
    return _id;
}

const std::vector<std::unique_ptr<GameObject>>& Scene::getObjects() const {
    return _objects;
}

void Scene::setId(const boost::uuids::uuid& uuid) {
    _id = uuid;
}
} // namespace dzemikk
