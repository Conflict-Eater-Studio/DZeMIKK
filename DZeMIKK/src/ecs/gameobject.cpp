#include "ecs/gameobject.h"

#include "ecs/componentRegistry.h"
#include "ecs/components/monoBehaviour.h"
#include "ecs/components/transform.h"
#include "ecs/scene.h"

#include <algorithm>

namespace dzemikk {
GameObject::GameObject() {
    _transform = addComponent<Transform>(TransformParams());
}

GameObject::~GameObject() {
    for (auto& component : _components) {
        if (component) {
            unregisterComponent(component.get());
        }
    }
}

Transform* GameObject::transform() {
    return _transform;
}

// --- Getters
GameObject* GameObject::getParent() const {
    return _parent;
}

const std::vector<GameObject*>& GameObject::getChildren() const {
    return _children;
}

std::string GameObject::getName() const {
    return _name;
}

const std::vector<MonoBehaviour*>& GameObject::getMonoBehaviours() const {
    return _monoBehaviours;
}

bool GameObject::hasStarted() const {
    return _hasStarted;
}

Scene const& GameObject::getScene() const {
    return *_scene;
}

// --- Setters
void GameObject::setName(const std::string& name) {
    _name = name;
}

void GameObject::setScene(Scene* scene) {
    _scene = scene;
    for (auto& mono : _monoBehaviours) {
        if (mono && _scene) {
            _scene->addPending(mono);
        }
    }
}

void GameObject::markStarted() {
    _hasStarted = true;
}

// --- Hierarchy operations
void GameObject::setParent(GameObject* parent) {
    if (_parent == parent) {
        return;
    }

    if (parent == this) {
        return;
    }

    // Reject cyclic parenting
    for (GameObject* ancestor = parent; ancestor; ancestor = ancestor->_parent) {
        if (ancestor == this) {
            return;
        }
    }

    // Remove from old parent
    if (_parent) {
        auto& siblings = _parent->_children;
        siblings.erase(std::ranges::remove(siblings, this).begin(), siblings.end());
    }

    _parent = parent;

    // Add to new parent
    if (_parent) {
        _parent->_children.push_back(this);
    }

    transform()->markDirty();
}

void GameObject::addChild(GameObject* child) {
    if (!child || child == this) {
        return;
    }
    child->setParent(this); // all logic lives in setParent
}

void GameObject::removeChild(GameObject* child) {
    detachChild(child);
}

void GameObject::detachChild(GameObject* child) {
    if (!child || child->getParent() != this) {
        return;
    }

    // setParent handles both sides of the parent/child relationship.
    child->setParent(nullptr);
}

void GameObject::detachChildren() {
    auto children = _children;
    for (auto* child : children) {
        if (child && child->getParent() == this) {
            child->setParent(nullptr);
        }
    }
}

void GameObject::destroyChild(GameObject* child) {
    if (!child || child->getParent() != this) {
        return;
    }

    child->setParent(nullptr);
    if (_scene) {
        _scene->destroyGameObject(child);
    }
}

void GameObject::destroyChildren() {
    auto children = _children;
    for (auto* child : children) {
        if (!child || child->getParent() != this) {
            continue;
        }

        child->setParent(nullptr);
        if (_scene) {
            _scene->destroyGameObject(child);
        }
    }
}

void GameObject::addScenePending(MonoBehaviour* mono) {
    if (_scene) {
        _scene->addPending(mono);
    }
}

void GameObject::removeSceneActive(MonoBehaviour* mono) {
    if (_scene) {
        _scene->removeActive(mono);
    }
}

void GameObject::registerComponent(Component* component) {
    ComponentRegistry::get().registerComponent(component);
}

void GameObject::unregisterComponent(Component* component) {
    ComponentRegistry::get().unregisterComponent(component);
}
} // namespace dzemikk
