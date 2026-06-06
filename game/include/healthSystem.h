#ifndef DZEMIKK_HEALTHSYSTEM_H
#define DZEMIKK_HEALTHSYSTEM_H

#include "ecs/components/monobehaviour.h"
#include "ecs/components/ui/uiTextRenderer.h"

#include <string>

namespace game {

class HealthSystem : public dzemikk::MonoBehaviour {
  public:
    using Base = dzemikk::MonoBehaviour;

    void start() override;

    void setHealth(float value);
    void setMaxHealth(float value, bool healToFull = false);

    void damage(float amount);
    void heal(float amount);

    void setTextRenderer(dzemikk::UITextRenderer* textRenderer);

    [[nodiscard]] float getMaxHealth() const;
    [[nodiscard]] float getCurrentHealth() const;
    [[nodiscard]] bool isDead() const;

    [[nodiscard]] std::string typeName() const override;

  private:
    void updateUI();

  private:
    float _maxHealth = 100.0f;
    float _currentHealth = 100.0f;

    dzemikk::UITextRenderer* _textRenderer = nullptr;
};

} // namespace game

#endif