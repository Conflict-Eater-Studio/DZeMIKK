#ifndef GAME_TERRITORY_PATTERN_REGISTRY_H
#define GAME_TERRITORY_PATTERN_REGISTRY_H

#include "enemySystem/territoryPattern.h"

#include <unordered_map>

namespace game {

/**
 * @brief Stores and provides access to registered territory patterns.
 *
 * Implements a singleton registry used to look up territory patterns
 * by name during enemy spawning and territory assignment.
 */
class TerritoryPatternRegistry {
  public:
    /**
     * @brief Returns the global registry instance.
     *
     * @return TerritoryPatternRegistry& Registry instance.
     */
    static TerritoryPatternRegistry& instance();

    /**
     * @brief Registers a territory pattern.
     *
     * @param pattern Pattern to register.
     */
    void registerPattern(const TerritoryPattern& pattern);

    /**
     * @brief Finds a territory pattern by name.
     *
     * @param name Pattern name.
     * @return const TerritoryPattern* Registered pattern or nullptr.
     */
    [[nodiscard]] const TerritoryPattern* get(const std::string& name) const;

  private:
#pragma region Storage

    std::unordered_map<std::string, TerritoryPattern> _patterns;

#pragma endregion
};

} // namespace game

#endif