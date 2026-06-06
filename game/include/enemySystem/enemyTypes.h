#ifndef GAME_ENEMY_TYPES_H
#define GAME_ENEMY_TYPES_H

namespace game {

/**
 * @brief Defines enemy behavioral tendencies.
 */
enum class EnemyPersonality : std::uint8_t { Aggressive, Defensive, Balanced };

/**
 * @brief Defines available enemy categories.
 */
enum class EnemyType : std::uint8_t { Normal, Special };

} // namespace game

#endif 