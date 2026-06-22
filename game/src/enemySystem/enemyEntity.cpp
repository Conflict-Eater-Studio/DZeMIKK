#include "enemySystem/enemyEntity.h"

void game::EnemyEntity::addTerritoryCell(HexCell* cell) {
    if (!cell) {
        return;
    }

    _territory.insert(cell);
}

const std::unordered_set<game::HexCell*> game::EnemyEntity::getTerritory() const {
    return _territory;
}

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
        setActionWeights({0.7F, 0.2F, 0.1F});
        break;

    case EnemyPersonality::Defensive:
        setActionWeights({0.3F, 0.5F, 0.2F});
        break;

    case EnemyPersonality::Balanced:
        setActionWeights({0.4F, 0.4F, 0.2F});
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

void game::EnemyEntity::setHp(double hp) {
    _hp = hp;
}

void game::EnemyEntity::setEnemyType(EnemyType type) {
    _type = type;
}

void game::EnemyEntity::setEnemyPersonality(EnemyPersonality personality) {
    _personality = personality;
}

double game::EnemyEntity::getHp() const {
    return _hp;
}

game::EnemyType game::EnemyEntity::getEnemyType() const {
    return _type;
}

game::EnemyPersonality game::EnemyEntity::getEnemyPersonality() const {
    return _personality;
}

void game::EnemyEntity::setActionWeights(const EnemyActionWeights& weights) {
    _actionWeights = weights;
    _actionWeights.normalize();
}

const game::EnemyActionWeights& game::EnemyEntity::getActionWeights() const {
    return _actionWeights;
}

void game::EnemyEntity::addBlockedCell(HexCell* cell) {
    if (!cell) {
        return;
    }

    _blockedEnemyCells.insert(cell);
}

const std::unordered_set<game::HexCell*> game::EnemyEntity::getBlockedCells() const {
    return _blockedEnemyCells;
}

void game::EnemyEntity::clearBlockedCells() {
    _blockedEnemyCells.clear();
}

bool game::EnemyEntity::isCellBlocked(HexCell* cell) const {
    return cell && _blockedEnemyCells.contains(cell);
}