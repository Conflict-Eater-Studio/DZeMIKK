#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#include "ecs/components/monobehaviour.h"
#include "map/HexCell.h"

namespace game {
class Entity : public dzemikk::MonoBehaviour {
  public:
    Entity() = default;

    [[nodiscard]] std::string typeName() const override {
        return "Entity";
    }

    virtual void onEnter(HexCell* cell) = 0;
    virtual void onExit() = 0;

    [[nodiscard]] const HexCell& getCell() const {
        return _cell;
    }
    HexCell& getCell() {
        return _cell;
    }

  private:
    HexCell _cell;
};
} // namespace game

#endif // GAME_ENTITY_H
