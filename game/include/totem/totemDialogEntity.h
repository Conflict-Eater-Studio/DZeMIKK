#ifndef GAME_TOTEM_DIALOG_ENTITY_H
#define GAME_TOTEM_DIALOG_ENTITY_H

#include "map/Entity.h"
namespace game {
class TotemDialogEntity : public Entity {
  public:
    TotemDialogEntity() = default;
    ~TotemDialogEntity() = default;

    /**
     * @brief Called when the entity is placed on a map cell.
     */
    void onEnter(HexCellPtr cell) override;

    /**
     * @brief Called when the entity is removed from the map.
     */
    void onExit() override;

    [[nodiscard]]
    std::string typeName() const override {
        return "TotemDialogEntity";
    }

    void use();

    void lightOff();

    void lightOn();
};
}// namespace game

#endif
