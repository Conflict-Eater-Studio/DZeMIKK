#include "map/PlayerEntity.h"

#include "ecs/gameobject.h"

namespace game {
void PlayerEntity::onEnter(HexCell* cell) {
    cell->setEntity(this);
    cell->setState(HexCell::State::Player);
    getOwner()->transform()->setPosition(cell->getCoord().toWorldPosition(0.65F, 0.0F) +
                                         glm::vec3(0.0F, 1.5F, 0.0F));
}

void PlayerEntity::onExit() {}

void PlayerEntity::tryMove(HexCell* targetCell) {
    if (targetCell->getEntity() != nullptr) {
        // Make the other entity perform its exit action
        targetCell->getEntity()->onExit();
    }
    switch (targetCell->getType()) {
    case HexCell::Type::Blocked:
        // If the cell is blocked we do nothing
        return;
    default:
        // Any other type of cell we just move into it
        onEnter(targetCell);
        break;
    }
}
} // namespace game
