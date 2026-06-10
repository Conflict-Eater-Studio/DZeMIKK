#pragma once

#include "map/Entity.h"

#include "totem/totemSpawnConfig.h"

class Game;

namespace game {

class TotemEntity : public Entity {
  public:
    TotemEntity() = default;
    ~TotemEntity() = default;
    void onEnter(HexCellPtr cell) override;
    void onExit() override;

    [[nodiscard]]
    std::string typeName() const override {
        return "TotemEntity";
    }

    void setConfig(const TotemSpawnConfig& cfg) {
        _config = cfg;
    }

    const TotemSpawnConfig& getConfig() const {
        return _config;
    }

    void setGame(Game* game) {
        _game = game;
    }

    void use();

  private:
    TotemSpawnConfig _config;
    bool _isUsed = false;
    Game* _game;
};

} // namespace game