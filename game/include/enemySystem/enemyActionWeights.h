#ifndef GAME_ENEMY_ACTION_WEIGHT_H
#define GAME_ENEMY_ACTION_WEIGHT_H
namespace game {

/**
 * @brief Weights used to determine the probability of enemy actions.
 *
 * The values represent relative chances of selecting attack,
 * defense, or heal actions. Call normalize() after modifying
 * any weight to ensure the total sum equals 1.0.
 */
struct EnemyActionWeights {

    /**
     * @brief Weight assigned to attack actions.
     */
    float attack = 0.33F;

    /**
     * @brief Weight assigned to defense actions.
     */
    float defense = 0.33F;

    /**
     * @brief Weight assigned to heal actions.
     */
    float heal = 0.34F;

    /**
     * @brief Normalizes all weights so their sum equals 1.0.
     *
     * If the sum of all weights is greater than zero, each weight
     * is divided by the total sum. Implementations may choose to
     * restore default values when the sum is zero or negative.
     */
    void normalize();
};

} // namespace game

#endif