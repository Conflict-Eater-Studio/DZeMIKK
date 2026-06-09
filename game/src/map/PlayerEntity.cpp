#include "map/PlayerEntity.h"

#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "game.h"
#include "gameStateMachine.h"
#include "healthSystem.h"
#include "map/ItemEntity.h"
#include "map/ItemEntityHealth.h"
#include "player/inventory.h"
#include "spdlog/spdlog.h"
#include "stateMachine/combatState.h"

namespace game {
void PlayerEntity::onEnter(HexCellPtr cell) {
    spdlog::info("Player entering cell at ({}, {})", cell->getCoord().q(), cell->getCoord().r());
    if (auto* ent = dynamic_cast<ItemEntity*>(cell->getEntity())) {
        spdlog::info("Player stepped on an item: {}", ent->typeName());
        if (_owner != nullptr && _owner->getScene() != nullptr) {
            spdlog::info("Player's scene is valid, applying item effect");
            switch (ent->getItemType()) {
            case ItemEntity::ItemType::Heal: {
                spdlog::info("Applying heal item effect");
                auto* playerHealth = _owner->getScene()
                                         ->findGameObjectByTag("PlayerHealthSystem")
                                         ->getComponent<game::HealthSystem>();
                float toHeal = dynamic_cast<ItemEntityHealth*>(ent)->getHealAmount();
                playerHealth->heal(toHeal);
                ent->consume();
                break;
            }
            case ItemEntity::ItemType::RevealPattern: {
                getOwner()->getComponent<Inventory>()->addItem(ItemEntity::ItemType::RevealPattern);
                ent->consume();
                break;
            }
            case ItemEntity::ItemType::RevealHex: {
                getOwner()->getComponent<Inventory>()->addItem(ItemEntity::ItemType::RevealHex);
                ent->consume();
                break;
            }
            case ItemEntity::ItemType::BonusHex:
                // Grant a bonus to the player
                // Implement the logic to grant a bonus here
                break;
            }
        }
    }

    cell->setEntity(this);
    cell->setState(HexCell::State::Player);
    setCell(cell);
    getOwner()->transform()->setPosition(
        cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
        glm::vec3(0.0F, 0.4F, 0.0F));
}

void PlayerEntity::onExit() {}

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

    if (!targetCell) {
        return;
    }

    if (targetCell->getEntity() != nullptr) {
        // Make the other entity perform its exit action
        targetCell->getEntity()->onExit();
    }

    if (getCell()) {
        getCell()->setEntity(nullptr);
        getCell()->setState(HexCell::State::Empty);
    }

    targetCell->setEntity(this);
    targetCell->setState(HexCell::State::Player);

    setCell(targetCell);

    getOwner()->transform()->setPosition(
        targetCell->getCoord().toWorldPosition(1.0F, 0.1F, targetCell->getHeight()) +
        glm::vec3(0.0F, 0.4F, 0.0F));
}
} // namespace game
