#include "ecs/component.h"
#include "ecs/gameobject.h"

namespace dzemikk {
    void Component::setOwner(GameObject* owner) {
        _owner = owner;
    }

    GameObject* Component::getOwner() const {
        return _owner;
    }
} // namespace dzemikk  