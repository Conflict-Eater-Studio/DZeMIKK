#ifndef GAME_ENEMY_PATTERN_COMPONENT_H
#define GAME_ENEMY_PATTERN_COMPONENT_H

#include "enemySystem/patternComponent.h"
#include "map/HexPattern.h"

namespace game {

/**
 * @brief Pattern component used by enemy AI.
 *
 * Extends the base pattern system with usage tracking,
 * allowing the game to collect statistics about which
 * patterns the enemy selects during combat.
 */
class EnemyPatternComponent : public PatternComponent {
  public:
    void start() override;
    void update(double deltaTime) override;
    void onDestroy() override;

    [[nodiscard]] std::string typeName() const override;

    /**
     * @brief Uses a pattern and records its usage.
     *
     * @param index Index of the pattern to activate.
     * @return True if the pattern was successfully used.
     */
    bool usePattern(size_t index) override;

    /**
     * @brief Clears all collected usage statistics.
     */
    void clearUsage();

    /**
     * @brief Registers usage of a pattern.
     *
     * @param pattern Pattern that was selected by the enemy.
     */
    void registerPatternUsage(const HexPattern* pattern);

    /**
     * @brief Returns how many times a pattern has been used.
     *
     * @param pattern Pattern to query.
     * @return Number of recorded usages.
     */
    [[nodiscard]] uint32_t getUsageCount(const HexPattern* pattern) const;

    /**
     * @brief Returns complete enemy pattern usage statistics.
     */
    [[nodiscard]] const std::unordered_map<const HexPattern*, uint32_t>& getPatternUsage() const;

  private:
    /**
     * @brief Usage count for each pattern instance.
     */
    std::unordered_map<const HexPattern*, uint32_t> _patternUsage;
};

} // namespace game

#endif