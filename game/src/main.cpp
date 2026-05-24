#include "assetManager/assetmanager.h"
#include "audio/audioManager.h"
#include "audio/sound.h"
#include "core/engine.h"
#include "game.h"
#include "utils/perlin.h"

#include <memory>
#include <tuple>

int main() {
    auto engine = std::make_unique<dzemikk::Engine>();

    auto game = new Game(engine.get());
    game->start();

    // engine->start();
    delete (game);
    return 0;
}
