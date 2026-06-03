#include "healthSystem.h"

#include <algorithm>

#include <ecs/gameobject.h>
#include <ecs/components/ui/uiTextRenderer.h>

namespace game {

void HealthSystem::start() {
    _currentHealth = std::clamp(_currentHealth, 0, _maxHealth);
}

void HealthSystem::setHealth(int value) {
    _currentHealth = std::clamp(value, 0, _maxHealth);
    if (_slider) {
        updateUI();
    }
    if (_currentHealth == 0) {
    }
}

void HealthSystem::setMaxHealth(int value, bool healToFull) {
    _maxHealth = std::max(1, value);
    if (healToFull) {
        _currentHealth = _maxHealth;
    } else {
        _currentHealth = std::min(_currentHealth, _maxHealth);
    }
    if (_slider) {
        _slider->setMaxValue(1.0F);
        _slider->setMinValue(0.0F);
        updateUI();
    }
}

void HealthSystem::damage(int amount) {
    if (amount <= 0 || isDead()) {
        return;
    }
    setHealth(_currentHealth - amount);
}

void HealthSystem::heal(int amount) {
    if (amount <= 0 || isDead()) {
        return;
    }
    setHealth(_currentHealth + amount);
}
void HealthSystem::setSlider(dzemikk::UISlider* slider) {
    _slider = slider;
}

int HealthSystem::getMaxHealth() const {
    return _maxHealth;
}

int HealthSystem::getCurrentHealth() const {
    return _currentHealth;
}

bool HealthSystem::isDead() const {
    return _currentHealth <= 0;
}

std::string HealthSystem::typeName() const {
    return "HealthSystem";
}

void HealthSystem::updateUI() {
    if (!_slider)
        return;

    float value = _currentHealth / static_cast<float>(_maxHealth);
    _slider->onValueChanged(value);

    auto* sliderGO = _slider->getOwner();

    if (!sliderGO)
        return;

    for (auto* child : sliderGO->getChildren()) {
        if (child->getName() == "Empty") {

            auto* text = child->getComponent<dzemikk::UITextRenderer>();
            if (text) {
                text->text = std::to_string(_currentHealth) + "/" + std::to_string(_maxHealth);
            }
        }
    }
}

} // namespace game