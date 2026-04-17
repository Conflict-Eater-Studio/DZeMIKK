#ifndef DZEMIKK_GAME_H
#define DZEMIKK_GAME_H
#include "core/engine.h"

class Game {
public:
    Game(dzemikk::Engine* engine) {
        this->engine = engine;
    }
    ~Game() = default;
    void init();
    void start();

  private:
    dzemikk::Engine* engine;

};

#endif