#include "totem/totemDialogEntity.h"

#include <ecs/gameobject.h>
#include <ecs/components/light/pointLight.h>

void game::TotemDialogEntity::onEnter(HexCellPtr cell) {
    if (!cell) {
        return;
    }

    if (getCell()) {
        onExit();
    }

    setCell(cell);

    cell->setEntity(this);
    cell->setState(HexCell::State::TotemDialog);

    getOwner()->transform()->setPosition(
        cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
        glm::vec3(0.0F, 1.0F, 0.0F));

    getOwner()->transform()->setScale({0.8, 0.8, 0.8});
}

void game::TotemDialogEntity::onExit() {
    if (getCell() == nullptr) {
        return;
    }

    getCell()->setEntity(nullptr);
    getCell()->setState(HexCell::State::Empty);

    setCell(nullptr);
}

void game::TotemDialogEntity::use() {
    lightOff();
}

void game::TotemDialogEntity::lightOff() {
    auto* lightGO = this->getOwner()->findChildByName("Light");
    auto* lightComp = lightGO->getComponent<dzemikk::PointLight>();
    lightComp->enabled(false);
}

void game::TotemDialogEntity::lightOn() {
    auto* lightGO = this->getOwner()->findChildByName("Light");
    auto* lightComp = lightGO->getComponent<dzemikk::PointLight>();
    lightComp->enabled(true);
}