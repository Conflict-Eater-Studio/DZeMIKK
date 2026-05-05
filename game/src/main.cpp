#include "core/engine.h"
#include "game.h"
#include <memory>
#include <tuple>

int main() {
    auto engine = std::make_unique<dzemikk::Engine>();
    auto game = Game(engine.get());
    game.start();
    engine->start();
    return 0;
}

