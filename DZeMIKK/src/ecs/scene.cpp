#include "ecs/scene.h"

#include "ecs/components/collider.h"
#include "ecs/components/monoBehaviour.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"
#include "renderer/mesh.h"
#include "renderer/model.h"
#include "scene/octree.h"

#include <algorithm>
#include <stack>
#include <unordered_set>

namespace dzemikk {
Scene::Scene() : _id(boost::uuids::random_generator()()) {};
Scene::~Scene() {
    for (const auto& object : _objects) {
        if (object->getParent() == nullptr) {
            object->destroy();
        }
    }

    _objects.clear();
    _pendingStart.clear();
    _active.clear();
    _activeSet.clear();
    _pendingDestroy.clear();
    _taggedObjects.clear();
    _namedObjects.clear();
    _idObjects.clear();
};

GameObject* Scene::createGameObject() {
    auto object = std::make_unique<GameObject>();
    GameObject* result = object.get();
    result->setScene(this);
    _objects.push_back(std::move(object));
    return result;
}

GameObject* Scene::createGameObject(const std::string& name) {
    auto object = std::make_unique<GameObject>();
    GameObject* result = object.get();
    result->setScene(this);
    object->setName(name);
    _objects.push_back(std::move(object));
    return result;
}

GameObject* Scene::createGameObject(const std::string& name, GameObject* parent) {
    GameObject* object = createGameObject(name);
    if (parent) {
        parent->addChild(object);
    }
    return object;
}

GameObject* Scene::createGameObject(GameObject* parent) {
    GameObject* object = createGameObject();
    if (parent) {
        parent->addChild(object);
    }
    return object;
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

    const auto activeSnapshot = _active;
    for (auto* mono : activeSnapshot) {
        if (mono == nullptr || !_activeSet.contains(mono)) {
            continue;
        }
        mono->update(deltaTime);
    }

    // Late Update all behaviours
    for (auto* mono : activeSnapshot) {
        if (mono == nullptr || !_activeSet.contains(mono)) {
            continue;
        }
        mono->lateUpdate(deltaTime);
    }

    processDelete();
}

void Scene::fixedUpdate(double deltaTime) {
    // Fixed update also starts components that haven't started since it runs at a fixed interval
    processPendingStart();

    const auto activeSnapshot = _active;
    for (auto* mono : activeSnapshot) {
        if (mono == nullptr || !_activeSet.contains(mono)) {
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

        std::erase_if(start,
                      [](MonoBehaviour* mono) { return mono == nullptr || mono->hasStarted(); });
        for (auto* mono : start) {
            mono->start();
            mono->markStarted();
            _active.push_back(mono);
            _activeSet.insert(mono);
        }
    }
}

void Scene::processDelete() {
    auto removeFromIndex = [](auto& index, GameObject* obj) {
        for (auto it = index.begin(); it != index.end();) {
            std::erase(it->second, obj);
            if (it->second.empty()) {
                it = index.erase(it);
            } else {
                ++it;
            }
        }
    };

    for (auto* obj : _pendingDestroy) {
        const auto& monos = obj->getMonoBehaviours();
        for (const auto& mono : monos) {
            mono->onDestroy();
        }
        auto inMonos = [&](MonoBehaviour* mono) {
            return std::ranges::find(monos, mono) != monos.end();
        };
        std::erase_if(_active, inMonos);
        for (auto* mono : monos) {
            _activeSet.erase(mono);
        }
        std::erase_if(_pendingStart, inMonos);

        if (obj->getParent()) {
            obj->getParent()->detachChild(obj);
        }

        std::erase_if(_objects, [obj](const auto& lObj) { return lObj.get() == obj; });
        removeFromIndex(_taggedObjects, obj);
        removeFromIndex(_idObjects, obj);
        removeFromIndex(_namedObjects, obj);
    }

    _pendingDestroy.clear();
}

void Scene::rebuildOctree() {
    _octree = std::make_unique<Octree>(glm::vec3(0.0f), 5000.0f); // Large bounds for the octree
    for (const auto& go : _objects) {
        auto* collider = go->getComponent<Collider>();
        if (collider && collider->isValid() && collider->getModel()) {
            glm::vec3 globalMin(FLT_MAX);
            glm::vec3 globalMax(-FLT_MAX);
            bool hasBounds = false;

            for (const auto& mesh : collider->getModel()->getSubMeshes()) {
                if (!mesh.mesh)
                    continue;
                globalMin = (glm::min)(globalMin, mesh.mesh->getBoundsMin());
                globalMax = (glm::max)(globalMax, mesh.mesh->getBoundsMax());
                hasBounds = true;
            }

            if (hasBounds) {
                const glm::mat4& worldMat = collider->getTransform()->getWorldMatrix();

                glm::vec3 corners[8] = {
                    glm::vec3(worldMat * glm::vec4(globalMin.x, globalMin.y, globalMin.z, 1.0f)),
                    glm::vec3(worldMat * glm::vec4(globalMax.x, globalMin.y, globalMin.z, 1.0f)),
                    glm::vec3(worldMat * glm::vec4(globalMin.x, globalMax.y, globalMin.z, 1.0f)),
                    glm::vec3(worldMat * glm::vec4(globalMax.x, globalMax.y, globalMin.z, 1.0f)),
                    glm::vec3(worldMat * glm::vec4(globalMin.x, globalMin.y, globalMax.z, 1.0f)),
                    glm::vec3(worldMat * glm::vec4(globalMax.x, globalMin.y, globalMax.z, 1.0f)),
                    glm::vec3(worldMat * glm::vec4(globalMin.x, globalMax.y, globalMax.z, 1.0f)),
                    glm::vec3(worldMat * glm::vec4(globalMax.x, globalMax.y, globalMax.z, 1.0f))};

                glm::vec3 worldMin(FLT_MAX);
                glm::vec3 worldMax(-FLT_MAX);
                for (int i = 0; i < 8; ++i) {
                    worldMin = (glm::min)(worldMin, corners[i]);
                    worldMax = (glm::max)(worldMax, corners[i]);
                }

                _octree->insert(go.get(), worldMin, worldMax);
            }
        }
    }
}

Octree* Scene::getOctree() const {
    return _octree.get();
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
    _activeSet.erase(mono);
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

void Scene::clearAllObjects() {
    for (auto& obj : _objects) {
        destroyGameObject(obj.get());
    }

    processDelete();
}

void Scene::registerTaggedObject(const std::string& tag, GameObject* object) {
    if (_taggedObjects.contains(tag) &&
        std::ranges::find(_taggedObjects[tag], object) != _taggedObjects[tag].end()) {
        return;
    }

    _taggedObjects[tag].push_back(object);
}

void Scene::unregisterTaggedObject(const std::string& tag, GameObject* object) {
    auto it = _taggedObjects.find(tag);
    if (it != _taggedObjects.end()) {
        std::erase(it->second, object);
        if (it->second.empty()) {
            _taggedObjects.erase(it);
        }
    }
}

void Scene::registerIdObject(const boost::uuids::uuid& id, GameObject* object) {
    if (_idObjects.contains(id) &&
        std::ranges::find(_idObjects[id], object) != _idObjects[id].end()) {
        return;
    }
    _idObjects[id].push_back(object);
}

void Scene::unregisterIdObject(const boost::uuids::uuid& id, GameObject* object) {
    auto it = _idObjects.find(id);
    if (it != _idObjects.end()) {
        std::erase(it->second, object);
        if (it->second.empty()) {
            _idObjects.erase(it);
        }
    }
}

GameObject* Scene::findGameObjectById(const boost::uuids::uuid& id) {
    return _idObjects.contains(id) && !_idObjects[id].empty() ? _idObjects[id].front() : nullptr;
}

std::vector<GameObject*> Scene::findGameObjectsById(const boost::uuids::uuid& id) {
    return _idObjects.contains(id) ? _idObjects[id] : std::vector<GameObject*>{};
}

GameObject* Scene::findGameObjectByName(const std::string& name) {
    return _namedObjects.contains(name) && !_namedObjects[name].empty()
               ? _namedObjects[name].front()
               : nullptr;
}

std::vector<GameObject*> Scene::findGameObjectsByName(const std::string& name) {
    return _namedObjects.contains(name) ? _namedObjects[name] : std::vector<GameObject*>{};
}

GameObject* Scene::findGameObjectByTag(const std::string& tag) {
    return _taggedObjects.contains(tag) && !_taggedObjects[tag].empty()
               ? _taggedObjects[tag].front()
               : nullptr;
}

std::vector<GameObject*> Scene::findGameObjectsByTag(const std::string& tag) {
    return _taggedObjects.contains(tag) ? _taggedObjects[tag] : std::vector<GameObject*>{};
}

void Scene::registerNamedObject(const std::string& name, GameObject* object) {
    if (_namedObjects.contains(name) &&
        std::ranges::find(_namedObjects[name], object) != _namedObjects[name].end()) {
        return;
    }

    _namedObjects[name].push_back(object);
}

void Scene::unregisterNamedObject(const std::string& name, GameObject* object) {
    if (_namedObjects.contains(name) && !_namedObjects[name].empty()) {
        std::erase(_namedObjects[name], object);
        if (_namedObjects[name].empty()) {
            _namedObjects.erase(name);
        }
    }
}

} // namespace dzemikk
