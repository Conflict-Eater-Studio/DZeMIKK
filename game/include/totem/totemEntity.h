#ifndef GAME_TOTEM_ENTITY_H
#define GAME_TOTEM_ENTITY_H

#include "map/Entity.h"

#include "totem/totemSpawnConfig.h"

class Game;

namespace game {

/**
 * @brief Interactive map entity that grants a one-time effect when used.
 *
 * Stores totem configuration data and tracks whether the totem
 * has already been activated.
 */
class TotemEntity : public Entity {
  public:
    TotemEntity() = default;
    ~TotemEntity() = default;

    /**
     * @brief Called when the entity is placed on a map cell.
     *
     * @param cell Cell occupied by the totem.
     */
    void onEnter(HexCellPtr cell) override;

    /**
     * @brief Called when the entity is removed from the map.
     */
    void onExit() override;

    [[nodiscard]]
    std::string typeName() const override {
        return "TotemEntity";
    }

    /**
     * @brief Assigns the spawn configuration for this totem.
     *
     * @param cfg Totem configuration data.
     */
    void setConfig(const TotemSpawnConfig& cfg) {
        _config = cfg;
    }

    /**
     * @brief Returns the configuration assigned to this totem.
     *
     * @return Totem spawn configuration.
     */
    const TotemSpawnConfig& getConfig() const {
        return _config;
    }

    /**
     * @brief Sets the owning game instance.
     *
     * @param game Game instance.
     */
    void setGame(Game* game) {
        _game = game;
    }

    /**
     * @brief Activates the totem effect.
     *
     * The effect can only be applied once. Subsequent calls
     * have no effect after the totem has been used.
     */
    void use();

  private:
    /**
     * @brief Totem configuration data.
     */
    TotemSpawnConfig _config;

    /**
     * @brief Indicates whether the totem has already been used.
     */
    bool _isUsed = false;

    /**
     * @brief Owning game instance.
     */
    Game* _game = nullptr;
};

} // namespace game

#endif