#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#include "ecs/components/monobehaviour.h"
#include "map/HexCell.h"

#include <memory>

namespace game {
class Entity : public dzemikk::MonoBehaviour {
  public:
    using HexCellPtr = std::shared_ptr<HexCell>;

    Entity() = default;

    [[nodiscard]] std::string typeName() const override {
        return "Entity";
    }
    virtual void onEnter(HexCellPtr cell) = 0;
    virtual void onExit() = 0;

    [[nodiscard]] const HexCellPtr& getCell() const {
        return _cell;
    }
    [[nodiscard]] HexCellPtr& getCell() {
        return _cell;
    }

  protected:
    void setCell(HexCellPtr cell) {
        _cell = std::move(cell);
    }

  private:
    HexCellPtr _cell{nullptr};
};
} // namespace game

#endif // GAME_ENTITY_H
