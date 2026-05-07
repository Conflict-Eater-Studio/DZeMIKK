#include "core/engine.h"
#include "game.h"
#include <memory>
#include <tuple>

#include "audio/audioManager.h"
#include "audio/sound.h"

#include "assetManager/assetmanager.h"

int main() {
    auto engine = std::make_unique<dzemikk::Engine>();
    
    auto game = new Game(engine.get());
    game->start();

    //engine->start();
    delete (game);
    return 0;
}

