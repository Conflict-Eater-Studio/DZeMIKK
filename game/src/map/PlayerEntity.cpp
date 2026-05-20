#include "map/PlayerEntity.h"

#include "ecs/gameobject.h"

namespace game {
void PlayerEntity::onEnter(HexCellPtr cell) {
    cell->setEntity(this);
    cell->setState(HexCell::State::Player);
    setCell(cell);
    getOwner()->transform()->setPosition(cell->getCoord().toWorldPosition(1.0F, 0.1F) +
                                         glm::vec3(0.0F, 1.5F, 0.0F));
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
void PlayerEntity::update(double dt) {
    Entity::update(dt);
    if (_path.empty() || _moveCount >= _path.size() - 1) {
        return;
    }

    _duration += dt;

    if (_duration > 0.1f && _moveCount < _path.size()) {
        HexCellPtr ptr = _path[_moveCount % _path.size()];
        tryMove(ptr);
        _duration = 0.0f;
        _moveCount++;
    }
}
void PlayerEntity::setPath(const std::vector<HexCellPtr>& path) {
    _path = path;
    _moveCount = 1;
}
} // namespace game
