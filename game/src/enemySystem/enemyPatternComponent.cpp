#include "enemySystem/enemyPatternComponent.h"

std::string game::EnemyPatternComponent::typeName() const {
    return "EnemyPatternComponent";
}

void game::EnemyPatternComponent::start() {
    // atk1
    addPattern(game::HexPattern({{0, 0}}, game::HexPattern::Type::ATK, 1.0F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}}, game::HexPattern::Type::ATK, 1.5F), -1);
    addPattern(game::HexPattern({{0, 0}, {2, -1}}, game::HexPattern::Type::ATK, 1.5F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {1, 0}}, game::HexPattern::Type::ATK, 2.0F), -1);
    addPattern(game::HexPattern({{0, 0}, {2, -1}, {1, 1}}, game::HexPattern::Type::ATK, 2.0F), -1);
    addPattern(game::HexPattern({{0, 0}, {2, -2}, {1, -1}}, game::HexPattern::Type::ATK, 2.0F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {2, -1}}, game::HexPattern::Type::ATK, 2.0F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {2, -2}, {0, 1}}, game::HexPattern::Type::ATK, 2.5F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, 0}, {2, 0}, {1, -1}}, game::HexPattern::Type::ATK, 2.5F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {2, -2}, {2, -1}}, game::HexPattern::Type::ATK, 2.5F),
        -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {1, 1}, {2, -1}}, game::HexPattern::Type::ATK, 2.5F),
        -1);
    addPattern(game::HexPattern({{0, 0}, {2, -1}, {2, -2}, {1, -1}}, game::HexPattern::Type::ATK, 2.5F),
        -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {0, 2}, {2, -2}}, game::HexPattern::Type::ATK, 2.5F),
        -1);

    // def1
    addPattern(game::HexPattern({{0, 0}}, game::HexPattern::Type::DEF), -1);
    addPattern(game::HexPattern({{0, 0}, {0, 1}}, game::HexPattern::Type::DEF, 1.5F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, 0}, {0, 1}}, game::HexPattern::Type::DEF, 2.0F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {-1, 1}}, game::HexPattern::Type::DEF, 2.0F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {-1, 0}}, game::HexPattern::Type::DEF, 2.0F), -1);
    addPattern(game::HexPattern({{0, 0}, {0, 1}, {1, -1}, {-1, 1}}, game::HexPattern::Type::DEF, 2.5F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {2, -2}, {-1, 1}}, game::HexPattern::Type::DEF, 2.5F), -1);
    addPattern(game::HexPattern({{0, 0}, {0, 1}, {0, -1}}, game::HexPattern::Type::DEF, 2.0F), 2.0F);
    addPattern(game::HexPattern({{0, 0}, {1, 0}, {2, 0}}, game::HexPattern::Type::DEF, 2.0F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {1, 1}, {2, 0}}, game::HexPattern::Type::DEF, 2.5F), -1);
    addPattern(game::HexPattern({{0, 0}, {-1, 1}, {-1, 0}, {0, 1}}, game::HexPattern::Type::DEF, 2.5F),
        -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {2, -1}, {2, -2}}, game::HexPattern::Type::DEF, 2.5F),
        -1);

    // hp1
    addPattern(game::HexPattern({{0, 0}}, game::HexPattern::Type::HEAL, 0.5F),
                                 -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}}, game::HexPattern::Type::HEAL, 0.75F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, 0}, {-1, 1}}, game::HexPattern::Type::HEAL, 1.0F), -1);
    addPattern(game::HexPattern({{0, 0}, {2, -2}}, game::HexPattern::Type::HEAL, 0.75F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {2, -2}}, game::HexPattern::Type::HEAL, 1.0F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {-1, 1}, {0, 2}}, game::HexPattern::Type::HEAL, 2.0F),
        -1);
    addPattern(game::HexPattern({{0, 0}, {0, 2}}, game::HexPattern::Type::HEAL, 0.75F), -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {2, -2}, {1, 0}}, game::HexPattern::Type::HEAL, 2.0F),
        -1);
    addPattern(game::HexPattern({{0, 0}, {-1, 1}, {-2, 2}, {0, 2}}, game::HexPattern::Type::HEAL, 2.0F),
        -1);
    addPattern(game::HexPattern({{0, 0}, {1, 0}, {2, -1}, {2, -2}}, game::HexPattern::Type::HEAL, 2.0F),
        -1);
    addPattern(game::HexPattern({{0, 0}, {1, -1}, {1, -2}, {2, -2}}, game::HexPattern::Type::HEAL, 2.0F),
        -1);
}

void game::EnemyPatternComponent::update(double deltaTime) {}

void game::EnemyPatternComponent::onDestroy() {}

bool game::EnemyPatternComponent::usePattern(size_t index) {
    return false;
}

void game::EnemyPatternComponent::registerPatternUsage(const HexPattern* pattern) {
    if (!pattern) {
        return;
    }

    _patternUsage[pattern]++;
}

void game::EnemyPatternComponent::clearUsage() {
    _patternUsage.clear();
}

uint32_t game::EnemyPatternComponent::getUsageCount(const HexPattern* pattern) const {
    auto it = _patternUsage.find(pattern);

    if (it == _patternUsage.end()) {
        return 0;
    }

    return it->second;
}

const std::unordered_map<const game::HexPattern*, uint32_t>&
game::EnemyPatternComponent::getPatternUsage() const {
    return _patternUsage;
}