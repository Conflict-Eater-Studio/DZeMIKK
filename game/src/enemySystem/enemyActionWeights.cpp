#include "enemySystem/enemyActionWeights.h"

void game::EnemyActionWeights::normalize() {
    float sum = attack + defense + heal;

    if (sum <= 0.0F) {
        attack = defense = heal = 1.0F / 3.0F;
        return;
    }

    attack /= sum;
    defense /= sum;
    heal /= sum;
}
