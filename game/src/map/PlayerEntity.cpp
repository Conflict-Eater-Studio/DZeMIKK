#include "map/PlayerEntity.h"

#include "ecs/gameobject.h"

namespace game {
void PlayerEntity::onEnter(HexCellPtr cell) {
    cell->setEntity(this);
    cell->setState(HexCell::State::Player);
    setCell(cell);
    getOwner()->transform()->setPosition(cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
                                         glm::vec3(0.0F, 0.4F, 0.0F));
}

void PlayerEntity::onExit() {
}

void PlayerEntity::tryMove(const HexCellPtr& targetCell) {
    if (!targetCell) {
        return;
    }

    if (targetCell->getEntity() != nullptr) {
        // Make the other entity perform its exit action
        targetCell->getEntity()->onExit();
    }
    switch (targetCell->getType()) {
    default:
        if (getCell() != nullptr) {
            getCell()->setEntity(nullptr);
            getCell()->setState(HexCell::State::Empty);
        }
        onEnter(targetCell);

        break;
    }
}
void PlayerEntity::teleportTo(const HexCellPtr& targetCell) {

    if (!targetCell)
        return;

    if (getCell()) {
        getCell()->setEntity(nullptr);
        getCell()->setState(HexCell::State::Empty);
    }

    targetCell->setEntity(this);
    targetCell->setState(HexCell::State::Player);

    setCell(targetCell);

    getOwner()->transform()->setPosition(targetCell->getCoord().toWorldPosition(1.0F, 0.1F, targetCell->getHeight()) +
                                         glm::vec3(0.0F, 0.4F, 0.0F));
}
} // namespace game
