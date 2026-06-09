#include "totem/totemEntity.h"

void game::TotemEntity::onEnter(HexCellPtr cell) {
    if (!cell)
        return;

    if (getCell())
        onExit();

    setCell(cell);

    cell->setEntity(this);
    cell->setState(HexCell::State::Totem);
}

void game::TotemEntity::onExit() {
    if (!getCell())
        return;

    getCell()->setEntity(nullptr);
    getCell()->setState(HexCell::State::Empty);

    setCell(nullptr);
}