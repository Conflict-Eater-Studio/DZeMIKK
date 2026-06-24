#include "stateMachine/explorationState.h"

#include "camera/cameraController.h"
#include "ecs/components/postProcessEffect.h"
#include "ecs/scene.h"
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
}
} // namespace explorationStateSound

game::ExplorationState::ExplorationState(Game* game) : _game(game) {}

void game::ExplorationState::onEnter() {
    // HACK: For those who find this, i don't wanna explain XD
    if (_game && _game->isPendingRestart()) {
        if (auto* cameraGO = _game->getCurrentScene().get()->findGameObjectByName("Camera");
            cameraGO) {
            if (auto effects = cameraGO->getComponents<dzemikk::PostProcessEffect>();
                !effects.empty()) {
                for (auto& effect : effects) {
                    if (effect->getShader().getAssetPath() == "shaders/grayscale" &&
                        effect->isEnabled()) {
                        effect->setEnabled(false);
                    }
                }
            }
        }
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
