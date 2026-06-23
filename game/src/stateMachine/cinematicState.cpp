#include "stateMachine/cinematicState.h"

#include "camera/cameraController.h"
#include "game.h"

#include <assetManager/assetmanager.h>
#include <assetManager/soundHandler.h>
#include <audio/audioManager.h>
#include <ecs/components/ui/imageRenderer.h>
#include <ecs/components/ui/uiTextRenderer.h>
#include <ecs/gameobject.h>
#include <ecs/scene.h>
#include <fmod/fmod.hpp>
#include <iostream>

namespace cinematicSound {
FMOD::Channel* combatFMODChannel = nullptr;

struct SoundInitContext {
    dzemikk::AudioManager* audioManager;
};

void onMusicLoad(const dzemikk::AssetHandle<dzemikk::Sound>& sound, SoundInitContext& ctx) {
    combatFMODChannel =
        ctx.audioManager->play(*sound.get(), dzemikk::AudioManager::SoundType::Music, true);
}

void onSFXLoad(const dzemikk::AssetHandle<dzemikk::Sound>& sound, SoundInitContext& ctx) {
    ctx.audioManager->play(*sound.get(), dzemikk::AudioManager::SoundType::SFX, false);
}
} // namespace cinematicSound

void game::CinematicState::onEnter() {
    cinematicSound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
    dzemikk::AssetManager::AssetTask<dzemikk::Sound, cinematicSound::SoundInitContext> taskS;
    taskS.context = sCtx;
    taskS.onLoad = cinematicSound::onMusicLoad;
    _game->getEngine()->getAssetManager()->getAsync("audio/wartwa_na_czas_walki.wav", taskS);

    auto* scene = _game->getCurrentScene().get();

    scene->findGameObjectByName("Player_Avatar_Panel")->enabled(false);
    _game->getCameraController()->setMode(CameraController::Mode::Cinematic);

    scene->findGameObjectByName("Cinematic")->enabled(true);
    scene->findGameObjectByName("Cinematic")->findChildByName("EndScreen")->enabled(false);

    _textRenderer = scene->findGameObjectByName("Cinematic")
                        ->findChildByName("Text")
                        ->getComponent<dzemikk::UITextRenderer>();

    _timer = 0.0f;
    _textTimer = 0.0f;
    _fadeAlpha = 0.0f;

    auto* cam = _game->getCurrentScene().get()->findGameObjectByName("Camera");

    _startRot = cam->transform()->getRotation();

    _phase = CinematicPhase::FadeIn;
    _currentText.clear();
}

void game::CinematicState::onExit() {
    auto* playerAvatar =
        _game->getCurrentScene().get()->findGameObjectByName("Player_Avatar_Panel");
    playerAvatar->enabled(true);
    _game->getEngine()->getAudioManager()->stop(cinematicSound::combatFMODChannel);
}

void game::CinematicState::onUpdate(float dt) {
    _timer += dt;

    switch (_phase) {

    case CinematicPhase::FadeIn: {
        float t = _timer / _fadeStartTime;
        _fadeAlpha = glm::clamp(t, 0.0f, 1.0f);

        applyFade(_fadeAlpha);

        if (_fadeAlpha >= 1.0f) {
            _phase = CinematicPhase::Text1;
            _currentText = "You have defeated KAHUN";
            _textRenderer->text.clear();
            _textTimer = 0.0f;
        }
        break;
    }

    case CinematicPhase::Text1: {
        typeText(dt);

        if (_textRenderer->text == _currentText) {
            _textTimer += dt;
            if (_textTimer > 1.2f) {
                _phase = CinematicPhase::Wait1;
                _textTimer = 0.0f;
            }
        }
        break;
    }

    case CinematicPhase::Wait1: {
        _textTimer += dt;

        if (_textTimer > 0.5f) {
            _phase = CinematicPhase::Text2;
            _currentText = "But... did you save your son?";
            _textRenderer->text.clear();
            _textTimer = 0.0f;
        }
        break;
    }

    case CinematicPhase::Text2: {
        typeText(dt);

        if (_textRenderer->text == _currentText) {
            _textTimer += dt;
            if (_textTimer > 1.5f) {
                _phase = CinematicPhase::FadeOut;
                _textTimer = 0.0f;
            }
        }
        break;
    }

    case CinematicPhase::FadeOut: {
        float t = _textTimer / _fadeDuration;
        _textTimer += dt;

        _fadeAlpha = 1.0f - glm::clamp(t, 0.0f, 1.0f);
        applyFade(_fadeAlpha);

        if (t >= 1.0f) {
            _phase = CinematicPhase::CameraReveal;

            _textTimer = 0.0f;
            _game->getCurrentScene()
                .get()
                ->findGameObjectByName("Cinematic")
                ->findChildByName("Text")
                ->enabled(false);
        }
        break;
    }

    case CinematicPhase::CameraReveal: {
        _textTimer += dt;

        auto* cam = _game->getCurrentScene().get()->findGameObjectByName("Camera");

        auto* camTransform = cam->transform();

        glm::quat start = _startRot;

        glm::quat target = glm::quat(glm::vec3(glm::radians(-30.0f), glm::radians(-180.0f), 0.0f));

        float duration = 2.5f;
        float t = glm::clamp(_textTimer / duration, 0.0f, 1.0f);

        float smoothT = t * t * (3.0f - 2.0f * t);

        glm::quat rot = glm::slerp(start, target, smoothT);

        camTransform->setRotation(rot);

        if (t >= 1.0f) {
            auto* go = _game->getCurrentScene()
                           .get()
                           ->findGameObjectByName("Cinematic")
                           ->findChildByName("BG");

            if (!go)
                return;

            auto* img = go->getComponent<dzemikk::ImageRenderer>();
            img->setColor({0.0F, 0.0F, 0.0F, 0.0F});
            _game->getCurrentScene().get()->findGameObjectByName("Cinematic")->enabled(true);
            _game->getCurrentScene()
                .get()
                ->findGameObjectByName("Cinematic")
                ->findChildByName("EndScreen")
                ->enabled(false);
            _game->getCurrentScene()
                .get()
                ->findGameObjectByName("Cinematic")
                ->findChildByName("Text")
                ->enabled(false);
            _phase = CinematicPhase::FinalFade;
        }

        break;
    }

    case CinematicPhase::FinalFade: {
        _textTimer += dt;

        float duration = 4.0f;
        float t = glm::clamp(_textTimer / duration, 0.0f, 1.0f);

        float alpha = t;

        applyFade(alpha);

        if (t >= 1.0f) {
            _phase = CinematicPhase::Done;
        }

        break;
    }

    case CinematicPhase::Done:
        _game->getCurrentScene()
            .get()
            ->findGameObjectByName("Cinematic")
            ->findChildByName("EndScreen")
            ->enabled(true);
        break;
    }
}

void game::CinematicState::applyFade(float alpha) {
    auto* go =
        _game->getCurrentScene().get()->findGameObjectByName("Cinematic")->findChildByName("BG");

    if (!go)
        return;

    auto* img = go->getComponent<dzemikk::ImageRenderer>();
    if (!img)
        return;

    img->setColor({0.0f, 0.0f, 0.0f, alpha});
}

void game::CinematicState::typeText(float dt) {
    static float acc = 0.0f;
    acc += dt;

    const float speed = 0.1f;

    if (acc >= speed) {
        acc = 0.0f;

        if (_textRenderer->text.size() < _currentText.size()) {
            _textRenderer->text += _currentText[_textRenderer->text.size()];
        }
    }
}
