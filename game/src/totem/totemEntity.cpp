#include "totem/totemEntity.h"
#include "game.h"
#include "player/playerPatternComponent.h"
#include "ui/combatUIPanel.h"

#include <ecs/scene.h>
#include <ecs/gameobject.h>
#include <ecs/components/light/pointLight.h>

void game::TotemEntity::onEnter(HexCellPtr cell) {
    if (!cell) {
        return;
    }

    if (getCell()) {
        onExit();
    }

    setCell(cell);

    cell->setEntity(this);
    cell->setState(HexCell::State::Totem);
}

void game::TotemEntity::onExit() {
    if (!getCell()) {
        return;
    }

    getCell()->setEntity(nullptr);
    getCell()->setState(HexCell::State::Empty);

    setCell(nullptr);
}

void game::TotemEntity::use() {
    if (_isUsed) {
        return;
    }

    _isUsed = true;

    auto* playerGO = _game->getCurrentScene().get()->findGameObjectByName("Player");
    auto* patternComponent = playerGO->getComponent<game::PlayerPatternComponent>();
    patternComponent->addPattern(_config.pattern);

    auto* pattern = patternComponent->getPattern(patternComponent->getPatternCount() - 1);

    auto* playerPanel = this->getOwner()->getScene()->findGameObjectByName("Player_Panel");
    auto* combatPlayerPanel = playerPanel->getComponent<game::CombatUIPanel>();
    combatPlayerPanel->addPatternSlot(*pattern);

    auto* lightGO = this->getOwner()->findChildByName("Light");
    auto* lightComp = lightGO->getComponent<dzemikk::PointLight>();
    lightComp->enabled(false);
}
