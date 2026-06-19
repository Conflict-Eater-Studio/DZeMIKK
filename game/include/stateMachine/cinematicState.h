#ifndef DZEMIKK_CINEMATIC_GAMESTATE_H
#define DZEMIKK_CINEMATIC_GAMESTATE_H

#include "stateMachine/iGameState.h"    

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Game;

namespace dzemikk {
class UITextRenderer;
}

namespace game {

class CinematicState : public IGameState {
  public:
    CinematicState(Game* game) : _game(game) {}

    void onEnter() override;

    void onExit() override;

    void onUpdate(float dt) override;

    enum class CinematicPhase {
        FadeIn,
        Text1,
        Wait1,
        Text2,
        Wait2,
        FadeOut,
        CameraReveal,
        FinalFade, Done
    };

  private:
    Game* _game;

    float _timer = 0.0f;
    float _textTimer = 0.0f;

    float _fadeStartTime = 1.0f; 
    float _fadeDuration = 1.5f; 

    float _fadeAlpha = 0.0f;

    bool _fadeStarted = false;

    CinematicPhase _phase = CinematicPhase::FadeIn;

    std::string _currentText;
    dzemikk::UITextRenderer* _textRenderer = nullptr;

    glm::quat _startRot;

    void applyFade(float alpha);
    void typeText(float dt);
};

} // namespace game
#endif