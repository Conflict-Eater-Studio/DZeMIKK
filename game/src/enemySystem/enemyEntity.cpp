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

    switch (_personality) {

    case EnemyPersonality::Aggressive:
        setActionWeights({0.7f, 0.2f, 0.1f});
        break;

    case EnemyPersonality::Defensive:
        setActionWeights({0.2f, 0.6f, 0.2f});
        break;

    case EnemyPersonality::Balanced:
        setActionWeights({0.4f, 0.4f, 0.2f});
        break;
    }
}

void game::EnemyEntity::onExit() {
    if (getCell() == nullptr) {
        return;
    }

    getCell()->setEntity(nullptr);
    getCell()->setState(HexCell::State::Empty);

    setCell(nullptr);
}
