#pragma once

namespace game {

struct EnemyActionWeights {
    float attack = 0.33f;
    float defense = 0.33f;
    float heal = 0.34f;

    void normalize();
};

} // namespace game