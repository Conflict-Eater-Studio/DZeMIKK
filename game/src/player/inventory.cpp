#include "player/inventory.h"

#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "game.h"
#include "gameStateMachine.h"
#include "map/ItemEntity.h"
#include "stateMachine/combatState.h"

#include <assetManager/assetmanager.h>
#include <audio/audioManager.h>
#include <audio/sound.h>

namespace inventorySound {
struct SoundInitContext {
    dzemikk::AudioManager* audioManager;
};

void onSFXLoad(const dzemikk::AssetHandle<dzemikk::Sound>& sound, SoundInitContext& ctx) {
    ctx.audioManager->play(*sound.get(), dzemikk::AudioManager::SoundType::SFX, false);
    ctx.audioManager->getSFXGroup()->setVolume(0.5F);
}
} // namespace inventorySound

namespace game {
void Inventory::addItem(ItemEntity::ItemType item, unsigned int count) {
    _items[item] += count;

    if (_game != nullptr) {
        dzemikk::GameObject* btn = nullptr;
        std::string newText;
        switch (item) {
        case ItemEntity::ItemType::RevealPattern: {
#if DZEMIKK_DEV_TOOLS
            spdlog::info("[Inventory] Added RevealPattern item. Total: {}", _items[item]);
#endif
            btn = _game->getCurrentScene().get()->findGameObjectByName("UI_RevealPatternBtn");
            newText = std::format("RP {}", _items[item]);
            break;
        }
        case ItemEntity::ItemType::RevealHex: {
#if DZEMIKK_DEV_TOOLS
            spdlog::info("[Inventory] Added RevealHex item. Total: {}", _items[item]);
#endif
            btn = _game->getCurrentScene().get()->findGameObjectByName("UI_RevealHexBtn");
            newText = std::format("RH {}", _items[item]);
            break;
        }
        default: {
            break;
        }
        }

        if (btn != nullptr) {
            btn->getChildren().at(0)->getComponent<dzemikk::UITextRenderer>()->text = newText;
        }
    }
}

void Inventory::tryUseItem(ItemEntity::ItemType item) {
    if (_game == nullptr) {
        return;
    }

    inventorySound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
    dzemikk::AssetManager::AssetTask<dzemikk::Sound, inventorySound::SoundInitContext> taskS;
    taskS.context = sCtx;
    taskS.onLoad = inventorySound::onSFXLoad;
    _game->getEngine()->getAssetManager()->getAsync("audio/prime_uzycie_itemu-Fmin.wav", taskS);

    if (_items.contains(item) && _items[item] > 0) {
        _items[item]--;
        std::string newText;
        dzemikk::GameObject* btn = nullptr;
        if (auto* cs = _game->getStateMachine()->getCurrentStateAs<CombatState>(); cs) {
            switch (item) {
            case ItemEntity::ItemType::RevealPattern: {
                btn = _game->getCurrentScene().get()->findGameObjectByName("UI_RevealPatternBtn");
                cs->revealRandomEnemyPattern();
                newText = std::format("RP {}", _items[item]);
                break;
            }
            case ItemEntity::ItemType::RevealHex: {
                btn = _game->getCurrentScene().get()->findGameObjectByName("UI_RevealHexBtn");
                cs->revealRandomEnemyCell();
                newText = std::format("RH {}", _items[item]);
                break;
            }
            default: {
                break;
            }
            }
        }

        if (btn != nullptr) {
            btn->getChildren().at(0)->getComponent<dzemikk::UITextRenderer>()->text = newText;
        }
    }
}
} // namespace game
