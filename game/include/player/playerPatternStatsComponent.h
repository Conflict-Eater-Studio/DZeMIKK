#ifndef GAME_PLAYER_PATTERN_STATS_COMPONENT_H
#define GAME_PLAYER_PATTERN_STATS_COMPONENT_H

#include "map/HexPattern.h"

#include <ecs/components/monobehaviour.h>

#include <cstdint>
#include <unordered_map>

namespace dzemikk {
class Engine;
}

namespace game {

/**
 * @brief Collects statistics about player pattern usage during combat.
 *
 * Tracks placed and removed patterns, total player actions,
 * and usage counts grouped by pattern type.
 */
class PlayerPatternStatsComponent : public dzemikk::MonoBehaviour {
  public:
    /**
     * @brief Aggregated pattern usage statistics.
     */
    struct Stats {

        /** Total number of placed patterns. */
        uint32_t placedTotal = 0;

        /** Total number of removed patterns. */
        uint32_t removedTotal = 0;

        /** Total number of pattern-related actions. */
        uint32_t actionsTotal = 0;

        /** Number of placed patterns grouped by type. */
        std::unordered_map<HexPattern::Type, uint32_t> placedByType;

        /** Number of removed patterns grouped by type. */
        std::unordered_map<HexPattern::Type, uint32_t> removedByType;

        /**
         * @brief Returns placement ratio for a pattern type.
         *
         * @param type Pattern type to query.
         * @return Ratio in range [0, 1].
         */
        [[nodiscard]] float getTypeRatio(HexPattern::Type type) const;
    };

    void start() override {}

    /**
     * @brief Updates statistics component.
     */
    void update(double dt) override;

    /**
     * @brief Registers placement of a pattern.
     *
     * Updates total counters and per-type statistics.
     */
    void registerPlacement(const HexPattern& pattern);

    /**
     * @brief Registers removal of a pattern.
     *
     * Updates total counters and per-type statistics.
     */
    void registerRemoval(const HexPattern& pattern);

    /**
     * @brief Returns current statistics.
     */
    [[nodiscard]] const Stats& getStats() const;

    /**
     * @brief Clears all collected statistics.
     */
    void reset();

    [[nodiscard]] std::string typeName() const override;

    /**
     * @brief Returns total number of placed patterns.
     */
    [[nodiscard]] uint32_t getTotalPlaced() const;

    /**
     * @brief Returns total number of removed patterns.
     */
    [[nodiscard]] uint32_t getTotalRemoved() const;

    /**
     * @brief Returns total number of tracked actions.
     */
    [[nodiscard]] uint32_t getTotalActions() const;

    /**
     * @brief Returns placement count for a pattern type.
     */
    [[nodiscard]] uint32_t getPlacedCountByType(HexPattern::Type type) const;

    /**
     * @brief Returns removal count for a pattern type.
     */
    [[nodiscard]] uint32_t getRemovedCountByType(HexPattern::Type type) const;

  private:
    /** Accumulated player pattern statistics. */
    Stats _stats;
};

} // namespace game

#endif