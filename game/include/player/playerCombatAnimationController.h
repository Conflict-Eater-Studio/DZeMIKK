#ifndef DZEMIKK_PLAYER_COMBAT_ANIMATION_CONTROLLER_H
#define DZEMIKK_PLAYER_COMBAT_ANIMATION_CONTROLLER_H

#include "ecs/components/monobehaviour.h"
#include "healthSystem.h"

namespace dzemikk {
class Animator;
}

namespace game {

class PlayerCombatAnimationController : public dzemikk::MonoBehaviour {
  public:
    void start() override;
    void update(double deltaTime) override;
    using Base = dzemikk::MonoBehaviour;

    [[nodiscard]] std::string typeName() const override {
        return "PlayerCombatAnimationController";
    }

    void setPlayerAnimator(dzemikk::Animator* animator);
    [[nodiscard]] dzemikk::Animator* getPlayerAnimator() const;

    void setHealthSystem(HealthSystem* healthSystem);
    [[nodiscard]] HealthSystem* getHealthSystem() const;

    void playConfirmRoundAttack() const;

  private:
    dzemikk::Animator* _playerAnimator = nullptr;
    HealthSystem* _playerHealthSystem = nullptr;
    dzemikk::Animator* _enemyAnimator = nullptr;
};

} // namespace game

#endif // DZEMIKK_PLAYER_COMBAT_ANIMATION_CONTROLLER_H
