#include "ecs/gameobject.h"
#include "ecs/components/monobehaviour.h"
#include "ecs/components/transform.h"
#include <algorithm>
#include <memory>

namespace dzemikk {
    GameObject::GameObject()
    : _transform(TransformParams{ .owner = this })
    {}

    Transform* GameObject::transform() {
        return &_transform;
    }

    const Transform* GameObject::transform() const {
        return &_transform;
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

    // --- Setters
    void GameObject::setName(const std::string& name) {
        _name = name;
    }

    // --- Hierarchy operations
    void GameObject::setParent(GameObject* parent) {
        if (_parent == parent) { return; }

        if (_parent) {
            auto& siblings = _parent->_children;
            siblings.erase(
                std::ranges::remove(siblings, this).begin(),
                siblings.end()
            );
        }

        _parent = parent;

        if (_parent) {
            _parent->addChild(this);
        }
    }

    void GameObject::addChild(GameObject* child) {
        if (!child || child == this) { return; }

        auto iter = std::ranges::find(_children, child);
        if (iter != _children.end()) { return; }

        _children.push_back(child);
        child->_parent = this;
    }

    void GameObject::removeChild(GameObject* child) {
        auto iter = std::ranges::find(_children,  child);
        if (iter != _children.end()) {
            _children.erase(iter);
        }
    }
}
