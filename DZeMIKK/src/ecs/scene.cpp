#include "ecs/scene.h"

#include "ecs/components/collider.h"
#include "ecs/components/monoBehaviour.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"
#include "scene/octree.h"
#include "renderer/model.h"
#include "renderer/mesh.h"

#include <algorithm>
#include <stack>
#include <unordered_set>

namespace dzemikk {
Scene::Scene() : _id(boost::uuids::random_generator()()) {};
Scene::~Scene() = default;

GameObject* Scene::createGameObject() {
    auto object = std::make_unique<GameObject>();
    GameObject* result = object.get();
    result->setScene(this);
    _objects.push_back(std::move(object));
    return result;
}

GameObject* Scene::createGameObject(const std::string& name) {
    GameObject* object = createGameObject();
    object->setName(name);
    return object;
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
                if (!mesh.mesh) continue;
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
                    glm::vec3(worldMat * glm::vec4(globalMax.x, globalMax.y, globalMax.z, 1.0f))
                };

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

GameObject* Scene::findGameObjectByName(const std::string& name) {
    auto it = std::ranges::find_if(
        _objects, [&name](const auto& obj) { return obj && obj->getName() == name; });

    return (it != _objects.end()) ? it->get() : nullptr;
}

} // namespace dzemikk
