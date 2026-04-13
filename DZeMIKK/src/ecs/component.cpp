#include "ecs/component.h"

#include "boost/uuid/uuid.hpp"
#include "ecs/gameobject.h"

namespace dzemikk {
Component::Component() : _id(boost::uuids::random_generator()()) {}

Component::Component(const boost::uuids::uuid& uuid) : _id(uuid) {}

void Component::setOwner(GameObject* owner) {
    _owner = owner;
}

void Component::setId(const boost::uuids::uuid& uuid) {
    _id = uuid;
}

GameObject* Component::getOwner() const {
    return _owner;
}

boost::uuids::uuid Component::getId() const {
    return _id;
}
} // namespace dzemikk
