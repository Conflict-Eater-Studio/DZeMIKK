#include "stateMachine/explorationState.h"

#include "camera/cameraController.h"
#include "game.h"

#include <assetManager/assetmanager.h>
#include <assetManager/soundHandler.h>
#include <audio/audioManager.h>
#include <audio/sound.h>

namespace explorationStateSound {
FMOD::Channel* FMODChannel = nullptr;

struct SoundInitContext {
    dzemikk::AudioManager* audioManager;
};

void onSoundLoad(const dzemikk::AssetHandle<dzemikk::Sound>& sound, SoundInitContext& ctx) {
    FMODChannel =
        ctx.audioManager->play(*sound.get(), dzemikk::AudioManager::SoundType::Music, true);
    ctx.audioManager->getMusicGroup()->setVolume(0.1F);
}
} // namespace explorationStateSound

game::ExplorationState::ExplorationState(Game* game) : _game(game) {}

void game::ExplorationState::onEnter() {
    if (_game && _game->isPendingRestart()) {
        _game->restartGame();
        return;
    }

    _game->getCameraController()->setMode(game::CameraController::Mode::Exploration);

    explorationStateSound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
    dzemikk::AssetManager::AssetTask<dzemikk::Sound, explorationStateSound::SoundInitContext> taskS;
    taskS.context = sCtx;
    taskS.onLoad = explorationStateSound::onSoundLoad;
    _game->getEngine()->getAssetManager()->getAsync(
        "audio/prime_przygodowka (loop, ale przyjemny).wav", taskS);
}

void game::ExplorationState::onExit() {
    _game->getEngine()->getAudioManager()->stop(explorationStateSound::FMODChannel);
}
