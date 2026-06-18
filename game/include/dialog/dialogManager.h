#ifndef GAME_DIALOG_MANAGER_H
#define GAME_DIALOG_MANAGER_H

#pragma once
#include "dialog/dialogEntity.h"
#include "ecs/components/monobehaviour.h"
#include "map/HexChunk.h"

class Game;

namespace dzemikk {
class AssetManager;
}

namespace game {

class World;

class DialogManager : public dzemikk::MonoBehaviour {
  public:
    using Base = dzemikk::MonoBehaviour;

    DialogManager() = default;

    void setWorld(World* world);
    void setAssetManager(dzemikk::AssetManager* assetManager);
    void setGame(Game* game);

    void addDialog(const DialogSpawnConfig& config);

    void update(double dt) override;

    [[nodiscard]] Dialog* getDialog(Entity* entity);
    [[nodiscard]] const Dialog* getDialog(Entity* entity) const;
    [[nodiscard]] bool isInDialog() const;

    [[nodiscard]] std::string typeName() const override {
        return "DialogManager";
    }

    [[nodiscard]] nlohmann::json saveState() const;
    void loadState(const nlohmann::json& j);
    void clear();
    void markDialogTriggered(const boost::uuids::uuid& targetEntityId);
    void markDialogUntriggered(const boost::uuids::uuid& targetEntityId);

  private:
    void attachDialog(Entity* entity, const DialogSpawnConfig& cfg);

    World* _world = nullptr;
    dzemikk::AssetManager* _assetManager = nullptr;
    Game* _game = nullptr;

    bool _inDialog = false;

    std::unordered_map<boost::uuids::uuid, DialogSpawnConfig> _configs;
    std::unordered_map<boost::uuids::uuid, Dialog> _dialogs;
};

} // namespace game

#endif // GAME_DIALOG_MANAGER_H
