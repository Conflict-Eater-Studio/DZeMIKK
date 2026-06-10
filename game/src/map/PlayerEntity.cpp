#include "map/PlayerEntity.h"

#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "game.h"
#include "gameStateMachine.h"
#include "healthSystem.h"
#include "map/ItemEntity.h"
#include "map/ItemEntityBonusHex.h"
#include "map/ItemEntityHealth.h"
#include "player/inventory.h"
#include "player/playerPatternComponent.h"
#include "spdlog/spdlog.h"
#include "stateMachine/combatState.h"
#include <ui/combatUIPanel.h>
#include <audio/sound.h>
#include <audio/audioManager.h>
#include <totem/totemEntity.h>

namespace playerEntitySound {
    struct SoundInitContext {
        dzemikk::AudioManager* audioManager;
    };

    void onSFXLoad(const dzemikk::AssetHandle<dzemikk::Sound>& sound, SoundInitContext& ctx) {
        ctx.audioManager->play(*sound.get(), dzemikk::AudioManager::SoundType::SFX, false);
        ctx.audioManager->getSFXGroup()->setVolume(0.5F);
    }
}

namespace game {
void PlayerEntity::onEnter(HexCellPtr cell) {
    if (auto* ent = dynamic_cast<ItemEntity*>(cell->getEntity())) {
        if (_owner != nullptr && _owner->getScene() != nullptr) {
            switch (ent->getItemType()) {
            case ItemEntity::ItemType::Heal: {
                auto* playerHealth = _owner->getScene()
                                         ->findGameObjectByTag("PlayerHealthSystem")
                                         ->getComponent<game::HealthSystem>();
                float toHeal = dynamic_cast<ItemEntityHealth*>(ent)->getHealAmount();
                playerHealth->heal(toHeal);
                ent->consume();

                playerEntitySound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
                dzemikk::AssetManager::AssetTask<dzemikk::Sound,
                                                 playerEntitySound::SoundInitContext>
                    taskS;
                taskS.context = sCtx;
                taskS.onLoad = playerEntitySound::onSFXLoad;
                _game->getEngine()->getAssetManager()->getAsync("audio/prime_uzycie_itemu-Fmin.wav",
                                                                taskS);
                break;
            }
            case ItemEntity::ItemType::RevealPattern: {
                getOwner()->getComponent<Inventory>()->addItem(ItemEntity::ItemType::RevealPattern);
                ent->consume();

                playerEntitySound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
                dzemikk::AssetManager::AssetTask<dzemikk::Sound,
                                                 playerEntitySound::SoundInitContext>
                    taskS;
                taskS.context = sCtx;
                taskS.onLoad = playerEntitySound::onSFXLoad;
                _game->getEngine()->getAssetManager()->getAsync(
                    "audio/prime_uzycie_itemu-Fmin.wav", taskS);
                break;
            }
            case ItemEntity::ItemType::RevealHex: {
                getOwner()->getComponent<Inventory>()->addItem(ItemEntity::ItemType::RevealHex);
                ent->consume();

                playerEntitySound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
                dzemikk::AssetManager::AssetTask<dzemikk::Sound,
                                                 playerEntitySound::SoundInitContext>
                    taskS;
                taskS.context = sCtx;
                taskS.onLoad = playerEntitySound::onSFXLoad;
                _game->getEngine()->getAssetManager()->getAsync("audio/prime_uzycie_itemu-Fmin.wav",
                                                                taskS);
                break;
            }
            case ItemEntity::ItemType::BonusHex:
                if (auto* ppc = getOwner()->getComponent<PlayerPatternComponent>(); ppc) {
                    HexPattern toAdd = dynamic_cast<ItemEntityBonusHex*>(ent)->getHexPattern();
                    ppc->addPattern(toAdd);
                    ent->consume();

                    auto pattern = ppc->getPattern(ppc->getPatternCount() - 1);

                    auto playerPanel =
                        this->getOwner()->getScene()->findGameObjectByName("Player_Panel");
                    auto combatPlayerPanel = playerPanel->getComponent<game::CombatUIPanel>();
                    combatPlayerPanel->addPatternSlot(*pattern);

                    playerEntitySound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
                    dzemikk::AssetManager::AssetTask<dzemikk::Sound,
                                                     playerEntitySound::SoundInitContext>
                        taskS;
                    taskS.context = sCtx;
                    taskS.onLoad = playerEntitySound::onSFXLoad;
                    _game->getEngine()->getAssetManager()->getAsync(
                        "audio/prime_uzycie_itemu-Fmin.wav", taskS);
                }
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
    
    if (auto* totem = dynamic_cast<TotemEntity*>(targetCell->getEntity())) {
        totem->use();
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
