#include "map/PlayerEntity.h"

#include "dialog/dialogManager.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "game.h"
#include "gameStateMachine.h"
#include "healthSystem.h"
#include "map/HexCoord.h"
#include "map/ItemEntity.h"
#include "map/ItemEntityBonusHex.h"
#include "map/ItemEntityHealth.h"
#include "player/inventory.h"
#include "player/playerPatternComponent.h"
#include "stateMachine/combatState.h"

#include <audio/audioManager.h>
#include <audio/sound.h>
#include <totem/totemDialogEntity.h>
#include <totem/totemEntity.h>
#include <ui/combatUIPanel.h>

namespace playerEntitySound {
struct SoundInitContext {
    dzemikk::AudioManager* audioManager;
};

void onSFXLoad(const dzemikk::AssetHandle<dzemikk::Sound>& sound, SoundInitContext& ctx) {
    ctx.audioManager->play(*sound.get(), dzemikk::AudioManager::SoundType::SFX, false);
}
} // namespace playerEntitySound

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

                if (!_hasSeenHealingItem) {
                    auto rp = _game->getCurrentScene().get()->findGameObjectByName(
                        "Reveal_Pattern_Found");
                    auto rh =
                        _game->getCurrentScene().get()->findGameObjectByName("Reveal_Hex_Found");
                    auto bh =
                        _game->getCurrentScene().get()->findGameObjectByName("Bonus_Hex_Found");
                    auto h = _game->getCurrentScene().get()->findGameObjectByName("Heal_Found");

                    rp->enabled(false);
                    rh->enabled(false);
                    bh->enabled(false);
                    h->enabled(true);

                    _hasSeenHealingItem = true;
                }

                break;
            }
            case ItemEntity::ItemType::RevealPattern: {
                getOwner()->getComponent<Inventory>()->addItem(ItemEntity::ItemType::RevealPattern,
                                                               5);
                ent->consume();

                playerEntitySound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
                dzemikk::AssetManager::AssetTask<dzemikk::Sound,
                                                 playerEntitySound::SoundInitContext>
                    taskS;
                taskS.context = sCtx;
                taskS.onLoad = playerEntitySound::onSFXLoad;
                _game->getEngine()->getAssetManager()->getAsync("audio/prime_uzycie_itemu-Fmin.wav",
                                                                taskS);

                if (!_hasSeenRevealPatternItem) {
                    auto rp = _game->getCurrentScene().get()->findGameObjectByName(
                        "Reveal_Pattern_Found");
                    auto rh =
                        _game->getCurrentScene().get()->findGameObjectByName("Reveal_Hex_Found");
                    auto bh =
                        _game->getCurrentScene().get()->findGameObjectByName("Bonus_Hex_Found");
                    auto h = _game->getCurrentScene().get()->findGameObjectByName("Heal_Found");

                    rp->enabled(true);
                    rh->enabled(false);
                    bh->enabled(false);
                    h->enabled(false);

                    _hasSeenRevealPatternItem = true;
                }
                break;
            }
            case ItemEntity::ItemType::RevealHex: {
                getOwner()->getComponent<Inventory>()->addItem(ItemEntity::ItemType::RevealHex, 5);
                ent->consume();

                playerEntitySound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
                dzemikk::AssetManager::AssetTask<dzemikk::Sound,
                                                 playerEntitySound::SoundInitContext>
                    taskS;
                taskS.context = sCtx;
                taskS.onLoad = playerEntitySound::onSFXLoad;
                _game->getEngine()->getAssetManager()->getAsync("audio/prime_uzycie_itemu-Fmin.wav",
                                                                taskS);

                if (!_hasSeenRevealHexItem) {
                    auto rp = _game->getCurrentScene().get()->findGameObjectByName(
                        "Reveal_Pattern_Found");
                    auto rh =
                        _game->getCurrentScene().get()->findGameObjectByName("Reveal_Hex_Found");
                    auto bh =
                        _game->getCurrentScene().get()->findGameObjectByName("Bonus_Hex_Found");
                    auto h = _game->getCurrentScene().get()->findGameObjectByName("Heal_Found");

                    rp->enabled(false);
                    rh->enabled(true);
                    bh->enabled(false);
                    h->enabled(false);

                    _hasSeenRevealHexItem = true;
                }
                break;
            }
            case ItemEntity::ItemType::BonusHex:
                if (auto* ppc = getOwner()->getComponent<PlayerPatternComponent>(); ppc) {
                    HexPattern toAdd = dynamic_cast<ItemEntityBonusHex*>(ent)->getHexPattern();
                    ppc->addPattern(toAdd);
                    ent->consume();

                    auto* pattern = ppc->getPattern(ppc->getPatternCount() - 1);

                    auto* playerPanel =
                        this->getOwner()->getScene()->findGameObjectByName("Player_Panel");
                    auto* combatPlayerPanel = playerPanel->getComponent<game::CombatUIPanel>();
                    combatPlayerPanel->refresh();
                    // combatPlayerPanel->addPatternSlot(*pattern);

                    playerEntitySound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
                    dzemikk::AssetManager::AssetTask<dzemikk::Sound,
                                                     playerEntitySound::SoundInitContext>
                        taskS;
                    taskS.context = sCtx;
                    taskS.onLoad = playerEntitySound::onSFXLoad;
                    _game->getEngine()->getAssetManager()->getAsync(
                        "audio/prime_uzycie_itemu-Fmin.wav", taskS);
                }

                if (!_hasSeeBonusHexItem) {
                    auto rp = _game->getCurrentScene().get()->findGameObjectByName(
                        "Reveal_Pattern_Found");
                    auto rh =
                        _game->getCurrentScene().get()->findGameObjectByName("Reveal_Hex_Found");
                    auto bh =
                        _game->getCurrentScene().get()->findGameObjectByName("Bonus_Hex_Found");
                    auto h = _game->getCurrentScene().get()->findGameObjectByName("Heal_Found");

                    rp->enabled(false);
                    rh->enabled(false);
                    bh->enabled(true);
                    h->enabled(false);

                    _hasSeeBonusHexItem = true;
                }

                break;
            }
        }
    }

    if (cell->getType() == HexCell::Type::BridgeHighlighted) {
        auto* hg = _game->getHexGrid();
        boost::uuids::uuid id = boost::uuids::nil_uuid();
        for (const auto& [uuid, bpi] : hg->getBlockingPatterns()) {
            for (const auto& coord : bpi.occupiedCoords) {
                if (coord == cell->getCoord()) {
                    id = bpi.blockedChunkId;
                    break;
                }
            }
        }
        if (id != boost::uuids::nil_uuid()) {
            const auto* bpi = hg->getBlockingPatternInfo(id);
            for (const auto coord : bpi->occupiedCoords) {
                auto* c = hg->getCell(coord).get();
                if (c) {
                    c->setType(HexCell::Type::Bridge);
                    c->setDirty(true);
                }
            }
        }
    }

    auto neighbors = HexCoord::getNeighbors(cell->getCoord());
    auto* grid = _game->getHexGrid();
    if (grid) {
        auto* dialogManager =
            _owner->getScene()->findGameObjectByTag("DialogManager")->getComponent<DialogManager>();
        for (const auto& neighborCoord : neighbors) {
            auto* neighborCell = grid->getCell(neighborCoord).get();
            if (!neighborCell) {
                continue;
            }
            auto* neighborEntity = neighborCell->getEntity();
            if (!neighborEntity) {
                continue;
            }
            if (dialogManager) {
                if (auto* dialog = dialogManager->getDialog(neighborEntity)) {
                    if (!dialog->isTriggered()) {
#if DZEMIKK_DEV_TOOLS
                        spdlog::info("[PlayerEntity]: Triggering dialog from neighbour");
#endif
                        dialog->start();

                        if (static_cast<TotemDialogEntity*>(neighborEntity)) {
                            static_cast<TotemDialogEntity*>(neighborEntity)->use();
                        }
                    }
                }
            }
        }
    }

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[PlayerEntity] Entering cell at coord ({}, {})", cell->getCoord().q(),
                 cell->getCoord().r());
#endif

    setCell(cell);
    getCell()->setEntity(this);
    getCell()->setState(HexCell::State::Player);
    getOwner()->transform()->setPosition(
        getCell()->getCoord().toWorldPosition(1.0F, 0.1F, getCell()->getHeight()) +
        glm::vec3(0.0F, 0.4F, 0.0F));

    auto bridges = _game->getHexGrid()->getBridges();
    auto bridge = std::ranges::find_if(bridges.begin(), bridges.end(), [this](const auto& pair) {
        const auto& bridgeInfo = pair.second;
        return bridgeInfo.hexes.contains(getCell().get());
    });

    bool isCheckpoint = getCell()->isCheckpoint();

    if (isCheckpoint && bridge != bridges.end()) {
        bool bridgeCheckpointUsed = std::ranges::any_of(
            bridge->second.hexes, [](const auto& hex) { return hex->isCheckpointUsed(); });

        if (!bridgeCheckpointUsed) {
            std::ranges::for_each(bridge->second.hexes,
                                  [](const auto& hex) { hex->setCheckpointUsed(true); });

            auto* worldGo = _game->getCurrentScene().get()->findGameObjectByTag("World");
            if (worldGo) {
                auto* world = worldGo->getComponent<World>();
                if (world) {
                    world->saveToFile("./world.json");
#if DZEMIKK_DEV_TOOLS
                    spdlog::info("[PlayerEntity] Checkpoint reached. Saved world data");
#endif
                } else {
#if DZEMIKK_DEV_TOOLS
                    spdlog::warn(
                        "[PlayerEntity] World GameObject found, but World component is not. "
                        "Cannot save");
#endif
                }
            } else {
#if DZEMIKK_DEV_TOOLS
                spdlog::warn("[PlayerEntity] Checkpoint reached but World GameObject is not found. "
                             "Cannot save.");
#endif
            }
        }
    }
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
#if DZEMIKK_DEV_TOOLS
    spdlog::info("[PlayerEntity] Attempting to teleport to cell at coord ({}, {})",
                 targetCell->getCoord().q(), targetCell->getCoord().r());
#endif

    if (!targetCell) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[PlayerEntity] Attempted to teleport to a null cell pointer");
#endif
        return;
    }

    if (targetCell->getEntity() != nullptr) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[PlayerEntity] Attempted to teleport to a cell occupied by another entity. "
                     "Calling onExit on the existing entity.");
#endif
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
        getCell()->getCoord().toWorldPosition(1.0F, 0.1F, getCell()->getHeight()) +
        glm::vec3(0.0F, 0.4F, 0.0F));
}
} // namespace game
