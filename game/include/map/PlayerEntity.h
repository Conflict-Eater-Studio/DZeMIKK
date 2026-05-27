#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#include "Entity.h"

namespace game {
class PlayerEntity : public Entity {
  public:
    PlayerEntity() = default;

    void onEnter(HexCellPtr cell) override;
    void onExit() override;

    [[nodiscard]] std::string typeName() const override {
        return "PlayerEntity";
    }

    void tryMove(const HexCellPtr& targetCell);

    void addTerritoryCell(HexCell* cell) {
        if (!cell)
            return;

        _territory.insert(cell);
    }

    const auto& getTerritory() const {
        return _territory;
    }

  private:
    std::unordered_set<HexCell*> _territory;
};
} // namespace game

#endif // GAME_PLAYER_H
