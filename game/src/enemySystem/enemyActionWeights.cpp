#include "enemySystem/enemyActionWeights.h"

void game::EnemyActionWeights::normalize() {
    float sum = attack + defense + heal;

    if (sum <= 0.0f) {
        attack = defense = heal = 1.0f / 3.0f;
        return;
    }

    attack /= sum;
    defense /= sum;
    heal /= sum;
}
