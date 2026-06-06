#include "healthSystem.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace game {

void HealthSystem::start() {
    _currentHealth = std::clamp(_currentHealth, 0.0f, _maxHealth);

    updateUI();
}

void HealthSystem::setHealth(float value) {
    _currentHealth = std::clamp(value, 0.0f, _maxHealth);

    updateUI();
}

void HealthSystem::setMaxHealth(float value, bool healToFull) {
    _maxHealth = std::max(1.0f, value);

    if (healToFull) {
        _currentHealth = _maxHealth;
    } else {
        _currentHealth = std::min(_currentHealth, _maxHealth);
    }

    updateUI();
}

void HealthSystem::damage(float amount) {
    if (amount <= 0.0f || isDead()) {
        return;
    }

    setHealth(_currentHealth - amount);
}

void HealthSystem::heal(float amount) {
    if (amount <= 0.0f || isDead()) {
        return;
    }

    setHealth(_currentHealth + amount);
}

void HealthSystem::setTextRenderer(dzemikk::UITextRenderer* textRenderer) {

    _textRenderer = textRenderer;
    updateUI();
}

float HealthSystem::getMaxHealth() const {
    return _maxHealth;
}

float HealthSystem::getCurrentHealth() const {
    return _currentHealth;
}

bool HealthSystem::isDead() const {
    return _currentHealth <= 0.0f;
}

std::string HealthSystem::typeName() const {
    return "HealthSystem";
}

void HealthSystem::updateUI() {
    if (!_textRenderer) {
        return;
    }

    std::ostringstream ss;

    ss << std::fixed << std::setprecision(1) << _currentHealth << "/"
       << static_cast<int>(std::round(_maxHealth));

    _textRenderer->text = ss.str();
}

} // namespace game