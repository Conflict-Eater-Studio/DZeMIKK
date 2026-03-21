#include "ecs/gameobject.h"
#include "ecs/components/transform.h"
#include <memory>

namespace dzemikk {
    GameObject::GameObject() {
        _transform = Transform();
    }

    Transform GameObject::transform() {
        return _transform;
    }
}
