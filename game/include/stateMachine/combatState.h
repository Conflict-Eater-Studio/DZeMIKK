#ifndef DZEMIKK_COMBAT_GAMESTATE_H
#define DZEMIKK_COMBAT_GAMESTATE_H

#include "stateMachine/iGameState.h"
#include <vector>

class Game;

namespace game {
class PlayerEntity;
class HexChunk;
class HexGrid;
class HexCoord;

class CombatState : public IGameState {
  public:
    CombatState(Game* game) : _game(game) {}

    void onEnter() override;

    void onExit() override;

    void onUpdate(float dt) override;

  private:
    Game* _game;

    void assignPlayerTerritory(game::PlayerEntity* player, HexChunk* chunk, HexGrid* grid,
                               const std::vector<HexCoord>& offsets, const HexCoord& center);
};

} // namespace game
#endif