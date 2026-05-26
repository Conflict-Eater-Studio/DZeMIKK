#ifndef GAME_ENEMY_ENTITY_H
#define GAME_ENEMY_ENTITY_H

#include "map/Entity.h"

namespace game {

class EnemyEntity : public Entity {
  public:
    EnemyEntity() = default;

    [[nodiscard]] std::string typeName() const override {
        return "EnemyEntity";
    }

    void onEnter(HexCellPtr cell) override;
    void onExit() override;
};

} // namespace game

#endif // GAME_ENEMY_ENTITY_H