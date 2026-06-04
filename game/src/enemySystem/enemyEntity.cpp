#include "enemySystem/enemyEntity.h"

void game::EnemyEntity::onEnter(HexCellPtr cell) {
    if (cell == nullptr) {
        return;
    }

    if (getCell() != nullptr) {
        onExit();
    }

    setCell(cell);

    cell->setEntity(this);
    cell->setState(HexCell::State::Enemy);
}

void game::EnemyEntity::onExit() {
    if (getCell() == nullptr) {
        return;
    }

    getCell()->setEntity(nullptr);
    getCell()->setState(HexCell::State::Empty);

    setCell(nullptr);
}
