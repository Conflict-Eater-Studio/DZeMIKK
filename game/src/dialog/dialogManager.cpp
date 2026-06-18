#include "dialog/dialogManager.h"

#include "game.h"
#include "map/HexGrid.h"
#include "scripts/world/world.h"

#include <assetManager/assetmanager.h>

namespace game {
void DialogManager::setWorld(World* world) {
    _world = world;
}

void DialogManager::setAssetManager(dzemikk::AssetManager* assetManager) {
    _assetManager = assetManager;
}

void DialogManager::setGame(Game* game) {
    _game = game;
}

void DialogManager::addDialog(const DialogSpawnConfig& config) {
    if (!_world || !_game) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[DialogManager] World/Game is not set. Cannot add dialog for entity: {}",
                     boost::uuids::to_string(config.targetEntityId));
#endif
        return;
    }

    _configs[config.targetEntityId] = config;

    Entity* targetEntity = nullptr;
    for (const auto& [chunkId, chunk] : _world->getGrid()->getChunks()) {
        if (!chunk) {
            continue;
        }
        for (const auto& [coord, cell] : chunk->getHexes()) {
            if (!cell) {
                continue;
            }
            auto* entity = cell->getEntity();
            if (entity && entity->getId() == config.targetEntityId) {
                targetEntity = entity;
                break;
            }
        }
        if (targetEntity) {
            break;
        }
    }

    if (!targetEntity) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[DialogManager] No entity found with id {}. Cannot attach dialog",
                     boost::uuids::to_string(config.targetEntityId));
#endif
        return;
    }

    attachDialog(targetEntity, config);
}

Dialog* DialogManager::getDialog(Entity* entity) {
    if (!entity) {
        return nullptr;
    }
    auto it = _dialogs.find(entity->getId());
    if (it == _dialogs.end()) {
        return nullptr;
    }
    return &it->second;
}

const Dialog* DialogManager::getDialog(Entity* entity) const {
    if (!entity) {
        return nullptr;
    }
    auto it = _dialogs.find(entity->getId());
    if (it == _dialogs.end()) {
        return nullptr;
    }
    return &it->second;
}

void DialogManager::attachDialog(Entity* entity, const DialogSpawnConfig& cfg) {
    if (!entity || !_game) {
        return;
    }

    auto result = _dialogs.emplace(entity->getId(), _game);
    auto& dialog = result.first->second;

    for (const auto& entry : cfg.entries) {
        dialog.enqueuePlan(entry.speaker, entry.text);
    }
}

void DialogManager::update(double dt) {
    if (!_game) {
        return;
    }

    _inDialog = false;
    for (auto& [entityId, dialog] : _dialogs) {
        if (dialog.update(static_cast<float>(dt))) {
            _inDialog = true;
        }
    }
}

nlohmann::json DialogManager::saveState() const {
    nlohmann::json j;
    for (const auto& [entityId, config] : _configs) {
        auto key = boost::uuids::to_string(entityId);
        j[key]["config"] = config;
        auto it = _dialogs.find(entityId);
        if (it != _dialogs.end()) {
            j[key]["triggered"] = it->second.isTriggered();
        }
    }
    return j;
}

void DialogManager::loadState(const nlohmann::json& j) {
    for (const auto& [idStr, dialogData] : j.items()) {
        auto entityId = boost::uuids::string_generator()(idStr);
        bool triggered = dialogData.value("triggered", false);

        if (!_dialogs.contains(entityId) && dialogData.contains("config")) {
            auto config = dialogData["config"].get<DialogSpawnConfig>();
            addDialog(config);
        }

        auto it = _dialogs.find(entityId);
        if (it != _dialogs.end()) {
            it->second.setTriggered(triggered);
        }
    }
}

void DialogManager::clear() {
    _dialogs.clear();
    _configs.clear();
}

void DialogManager::markDialogTriggered(const boost::uuids::uuid& targetEntityId) {
    auto it = _dialogs.find(targetEntityId);
    if (it != _dialogs.end()) {
        it->second.setTriggered(true);
    }
}

void DialogManager::markDialogUntriggered(const boost::uuids::uuid& targetEntityId) {
    auto it = _dialogs.find(targetEntityId);
    if (it != _dialogs.end()) {
        it->second.setTriggered(false);
    }
}

bool DialogManager::isInDialog() const {
    return _inDialog;
}

} // namespace game
