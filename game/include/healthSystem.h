#ifndef DZEMIKK_HEALTHSYSTEM_H
#define DZEMIKK_HEALTHSYSTEM_H

#include "ecs/components/monobehaviour.h"
#include "ecs/components/ui/uiSlider.h"

#include <string>

namespace game {

class HealthSystem : public dzemikk::MonoBehaviour {
public:
    using Base = dzemikk::MonoBehaviour;

    void start() override;

    void setHealth(int value);
    void setMaxHealth(int value, bool healToFull = false);
    void damage(int amount);
    void heal(int amount);
    void setSlider(dzemikk::UISlider* slider);

    [[nodiscard]] int getMaxHealth() const;
    [[nodiscard]] int getCurrentHealth() const;
    [[nodiscard]] bool isDead() const;

    [[nodiscard]] std::string typeName() const override;
private:
    int _maxHealth = 100;
    int _currentHealth = 100;
    dzemikk::UISlider* _slider = nullptr;

    void updateUI();
};

} // namespace game

#endif