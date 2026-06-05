#ifndef GAME_PATTERN_COMPONENT_H
#define GAME_PATTERN_COMPONENT_H

#include "map/HexPattern.h"

#include <ecs/components/monobehaviour.h>
#include <optional>
#include <vector>

namespace game {

/**
 * @brief Base component for managing pattern collections.
 *
 * Stores available patterns together with their usage counts and
 * provides utilities for adding, removing and querying patterns.
 */
class PatternComponent : public dzemikk::MonoBehaviour {
  public:
    /**
     * @brief Represents a pattern and its available count.
     */
    struct PatternEntry {
        HexPattern pattern;
        int count = 0;
    };

#pragma region Lifecycle

    void start() override = 0;
    void update(double deltaTime) override = 0;
    void onDestroy() override = 0;

#pragma endregion

#pragma region Pattern management

    /**
     * @brief Adds a pattern to the collection.
     *
     * @param pattern Pattern to add.
     * @param count Initial pattern count.
     */
    void addPattern(const HexPattern& pattern, int count = 1);

    /**
     * @brief Inserts a pattern at a specific position.
     *
     * @param index Target index.
     * @param pattern Pattern to insert.
     * @param count Initial pattern count.
     */
    void insertPattern(size_t index, const HexPattern& pattern, int count = 1);

    /**
     * @brief Removes a pattern by index.
     *
     * @param index Pattern index.
     * @return true If the pattern was removed.
     * @return false Otherwise.
     */
    bool removePattern(size_t index);

    /**
     * @brief Removes a pattern by value.
     *
     * @param pattern Pattern to remove.
     * @return true If the pattern was removed.
     * @return false Otherwise.
     */
    bool removePattern(const HexPattern& pattern);

    /**
     * @brief Removes all patterns from the collection.
     */
    void clearPatterns();

    /**
     * @brief Adds instances to a pattern count.
     *
     * @param index Pattern index.
     * @param amount Amount to add.
     * @return true If successful.
     * @return false Otherwise.
     */
    bool addCount(size_t index, int amount);

    /**
     * @brief Removes instances from a pattern count.
     *
     * @param index Pattern index.
     * @param amount Amount to remove.
     * @return true If successful.
     * @return false Otherwise.
     */
    bool removeCount(size_t index, int amount);

    /**
     * @brief Sets the available count for a pattern.
     *
     * @param index Pattern index.
     * @param count New count value.
     * @return true If successful.
     * @return false Otherwise.
     */
    bool setCount(size_t index, int count);

#pragma endregion

#pragma region Pattern usage

    /**
     * @brief Checks whether a pattern can currently be used.
     *
     * @param index Pattern index.
     * @return true If the pattern can be used.
     * @return false Otherwise.
     */
    [[nodiscard]] bool canUsePattern(size_t index) const;

    /**
     * @brief Uses a pattern.
     *
     * @param index Pattern index.
     * @return true If the pattern was successfully used.
     * @return false Otherwise.
     */
    virtual bool usePattern(size_t index) = 0;

#pragma endregion

#pragma region Pattern queries

    /**
     * @brief Returns the number of stored patterns.
     *
     * @return size_t Pattern count.
     */
    [[nodiscard]] size_t getPatternCount() const;

    /**
     * @brief Returns a pattern entry by index.
     *
     * @param index Pattern index.
     * @return PatternEntry* Pattern entry or nullptr.
     */
    PatternEntry* getPattern(size_t index);

    /**
     * @brief Returns a pattern entry by index.
     *
     * @param index Pattern index.
     * @return const PatternEntry* Pattern entry or nullptr.
     */
    [[nodiscard]] const PatternEntry* getPattern(size_t index) const;

    /**
     * @brief Returns all stored pattern entries.
     *
     * @return const std::vector<PatternEntry>& Pattern collection.
     */
    [[nodiscard]] const std::vector<PatternEntry>& getPatterns() const;

    /**
     * @brief Finds a pattern in the collection.
     *
     * @param pattern Pattern to find.
     * @return int Pattern index or -1 if not found.
     */
    [[nodiscard]] int findPattern(const HexPattern& pattern) const;

#pragma endregion

    [[nodiscard]] std::string typeName() const override;

  protected:
#pragma region Storage

    std::vector<PatternEntry> _patterns;

#pragma endregion
};

} // namespace game

#endif