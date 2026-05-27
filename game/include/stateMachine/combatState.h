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
class EnemyEntity;

class CombatState : public IGameState {
  public:
    CombatState(Game* game) : _game(game) {}

    void onEnter() override;

    void onExit() override;

    void onUpdate(float dt) override;

  private:
    Game* _game = nullptr;
    PlayerEntity* player = nullptr;
    EnemyEntity* _currentEnemy = nullptr;
};

} // namespace game
#endif