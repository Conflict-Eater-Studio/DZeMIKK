#include "ecs/gameobject.h"

#include "boost/uuid/random_generator.hpp"
#include "ecs/componentRegistry.h"
#include "ecs/components/monoBehaviour.h"
#include "ecs/components/transform.h"
#include "ecs/components/ui/rectTransform.h"
#include "ecs/scene.h"

#include <algorithm>

namespace dzemikk {
GameObject::GameObject() : _id(boost::uuids::random_generator()()) {
    _transform = addComponent<Transform>(TransformParams());
}

GameObject::GameObject(const boost::uuids::uuid& uuid) : _id(uuid) {
    _transform = addComponent<Transform>(TransformParams());
}

GameObject::~GameObject() {
    for (auto& component : _components) {
        if (component) {
            ComponentRegistry::get().unregisterComponent(component.get());
        }
    }
}

Transform* GameObject::transform() {
    if (!_transform && _rectTransform) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("[{}] GameObject '{}' is a UI element and has a RectTransform but "
                      "GameObject::transform() was called.",
                      boost::uuids::to_string(_id), _name);
#endif
        throw std::runtime_error("This GameObject is a UI element. Call rectTransform() instead");
    }

    return _transform;
}

const Transform* GameObject::transform() const {
    if (!_transform && _rectTransform) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("[{}] GameObject '{}' is a UI element and has a RectTransform but "
                      "GameObject::transform() was called.",
                      boost::uuids::to_string(_id), _name);
#endif
        throw std::runtime_error("This GameObject is a UI element. Call rectTransform() instead");
    }

    return _transform;
}

RectTransform* GameObject::rectTransform() {
    if (_transform && !_rectTransform) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("[{}] GameObject '{}' is a world object and has a Transform but "
                      "GameObject::rectTransform() was called.",
                      boost::uuids::to_string(_id), _name);
#endif
        throw std::runtime_error("This GameObject is a world object. Call transform() instead");
    }

    return _rectTransform;
}

const RectTransform* GameObject::rectTransform() const {
    if (_transform && !_rectTransform) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("[{}] GameObject '{}' is a world object and has a Transform but "
                      "GameObject::rectTransform() was called.",
                      boost::uuids::to_string(_id), _name);
#endif
        throw std::runtime_error("This GameObject is a world object. Call transform() instead");
    }

    return _rectTransform;
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

boost::uuids::uuid GameObject::getId() const {
    return _id;
}

const std::vector<MonoBehaviour*>& GameObject::getMonoBehaviours() const {
    return _monoBehaviours;
}

const std::vector<std::unique_ptr<Component>>& GameObject::getAllComponents() const {
    return _components;
}

bool GameObject::hasStarted() const {
    return _hasStarted;
}

Scene* GameObject::getScene() {
    return _scene;
}

bool GameObject::isEnabled() const {
    return _isEnabled;
}

// --- Setters
void GameObject::setName(const std::string& name) {
    _name = name;
}

void GameObject::setId(const boost::uuids::uuid& uuid) {
    _id = uuid;
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

    if (parent) {
        // If a parent has a RectTransform, then it's in a canvas space
        // We need to remove existing Transform and replace it with RectTransform
        if (!_rectTransform && parent->getComponent<RectTransform>()) {
            removeComponent(_transform);
            _transform = nullptr;
            _rectTransform = addComponent<RectTransform>();
        }

        // If a parent has a Transform, then it's in a world space
        // We need to remove existing RectTransform and replace it with Transform
        if (!_transform && parent->getComponent<Transform>()) {
            removeComponent(_rectTransform);
            _rectTransform = nullptr;
            _transform = addComponent<Transform>();
        }
    } else {
        // If we have no parent or set it to nullptr, we default to world space, so ensure we have a
        // Transform
        if (!_transform) {
            removeComponent(_rectTransform);
            _rectTransform = nullptr;
            _transform = addComponent<Transform>();
        }
    }

    if (_transform) {
        _transform->markDirty();
    }
    if (_rectTransform) {
        _rectTransform->markDirty();
    }
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

void GameObject::enabled(bool isEnabled) {
    _isEnabled = isEnabled;
    for (auto& component : _components) {
        if (component) {
            component->enabled(isEnabled);
        }
    }
    for (auto* child : _children) {
        if (child) {
            child->enabled(isEnabled);
        }
    }
}
} // namespace dzemikk
