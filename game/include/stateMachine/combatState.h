#ifndef DZEMIKK_COMBAT_GAMESTATE_H
#define DZEMIKK_COMBAT_GAMESTATE_H

#include "stateMachine/iGameState.h"

class Game;

namespace game {

class CombatState : public IGameState {
  public:
    CombatState(Game* game) : _game(game) {}

    void onEnter() override;

    void onExit() override;

    void onUpdate(float dt) override {};

  private:
    Game* _game;
};

} // namespace game
#endif