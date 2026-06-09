#pragma once

#include "map/Entity.h"

#include "totem/totemSpawnConfig.h"

namespace game {

class TotemEntity : public Entity {
  public:
    void onEnter(HexCellPtr cell) override;
    void onExit() override;

    [[nodiscard]]
    std::string typeName() const override {
        return "TotemEntity";
    }

  private:
    TotemSpawnConfig _config;

  public:
    void setConfig(const TotemSpawnConfig& cfg) {
        _config = cfg;
    }

    const TotemSpawnConfig& getConfig() const {
        return _config;
    }
};

} // namespace game