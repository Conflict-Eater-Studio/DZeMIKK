#include "ecs/components/monobehaviour.h"
#include "ecs/gameobject.h"

namespace dzemikk {
    // --- Start state management
    bool MonoBehaviour::hasStarted() const {
        return _started;
    }

    void MonoBehaviour::markStarted() {
        _started = true;
    }

    // --- Getters
    GameObject* MonoBehaviour::getOwner() const {
        return _owner;
    }

    // --- Setters
    void MonoBehaviour::setOwner(GameObject* owner) {
        _owner = owner;
    }

}
