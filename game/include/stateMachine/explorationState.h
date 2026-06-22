#ifndef DZEMIKK_EXPLORATION_GAMESTATE_H
#define DZEMIKK_EXPLORATION_GAMESTATE_H

#include "stateMachine/iGameState.h"

class Game;

namespace game {

class ExplorationState : public IGameState {
  public:
    ExplorationState(Game* game);

    void onEnter() override;

    void onExit() override;

    void onUpdate(float dt) override {};

  private:
    Game* _game;
};

} // namespace game
#endif
