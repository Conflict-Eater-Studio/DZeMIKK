#include "enemySystem/enemyPatternComponent.h"

std::string game::EnemyPatternComponent::typeName() const {
    return "EnemyPatternComponent";
}

void game::EnemyPatternComponent::start() {
    // atk1
    addPattern(HexPattern({{-1, 1}, {0, 0}, {1, -1}}, HexPattern::Type::ATK, 1.2F), 100);

    // atk2
    //addPattern(HexPattern({{-1, 0}, {0, 0}, {1, -1}}, HexPattern::Type::ATK, 1.2F), -1);

    // atk3
    //addPattern(HexPattern({{0, 0}, {1, -1}, {-1, 0}, {0, 1}}, HexPattern::Type::ATK, 1.3F), -1);

    // def1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::DEF), 100);

    // def2
    addPattern(HexPattern({{-1, 1}, {0, 0}, {1, -1}}, HexPattern::Type::DEF, 1.2F), 100);

    // hp1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::HEAL), 100);
}

void game::EnemyPatternComponent::update(double deltaTime) {}

void game::EnemyPatternComponent::onDestroy() {}

bool game::EnemyPatternComponent::usePattern(size_t index) {
    return false;
}
