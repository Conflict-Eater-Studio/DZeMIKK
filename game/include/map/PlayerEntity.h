#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#include "Entity.h"

class Game;

namespace game {
class HexCell;
class PlayerEntity : public Entity {
  public:
    using Base = Entity;

    PlayerEntity() = default;

    void setGame(Game* game) {
        _game = game;
    }
    [[nodiscard]] Game* getGame() const {
        return _game;
    }

    void onEnter(HexCellPtr cell) override;
    void onExit() override;

    [[nodiscard]] std::string typeName() const override {
        return "PlayerEntity";
    }

    void tryMove(const HexCellPtr& targetCell);
    void teleportTo(const HexCellPtr& targetCell);

    void addTerritoryCell(HexCell* cell) {
        if (!cell) {
            return;
        }

        _territory.insert(cell);
    }

    [[nodiscard]] const auto& getTerritory() const {
        return _territory;
    }

    void clearTerritory() {
        _territory.clear();
    }

  private:
    Game* _game = nullptr;
    std::unordered_set<HexCell*> _territory;
};
} // namespace game

#endif // GAME_PLAYER_H
