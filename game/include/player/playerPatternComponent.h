#ifndef PLAYER_PATTERN_COMPONENT_H
#define PLAYER_PATTERN_COMPONENT_H

#include "enemySystem/patternComponent.h"
#include "map/HexPattern.h"

#include <optional>
#include <vector>

namespace dzemikk {
class Engine;
}

namespace game {

class PlayerEntity;
class HexGrid;
class PlayerPatternStatsComponent;

/**
 * @brief Handles player pattern selection, placement and preview logic.
 *
 * Extends PatternComponent with gameplay functionality for placing
 * patterns on the grid, managing previews and tracking placed patterns.
 */
class PlayerPatternComponent : public PatternComponent {
  public:
    /**
     * @brief Represents a pattern placed on the grid.
     */
    struct PlacedPattern {
        HexPattern pattern;
        HexCoord origin;
        std::vector<dzemikk::GameObject*> objects;
    };

    using ListenerID = uint32_t;

#pragma region Lifecycle

    void start() override;
    void update(double deltaTime) override;
    void onDestroy() override;

#pragma endregion

#pragma region Pattern management

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

    /**
     * @brief Activates a pattern for placement.
     *
     * @param index Pattern index.
     * @return true If successful.
     * @return false Otherwise.
     */
    bool usePattern(size_t index) override;

#pragma endregion

#pragma region Configuration

    /**
     * @brief Sets the engine instance used by the component.
     *
     * @param engine Engine instance.
     */
    void setEngine(dzemikk::Engine* engine);

    /**
     * @brief Sets the target grid used for pattern placement.
     *
     * @param grid Target grid.
     */
    void setGrid(game::HexGrid* grid);

#pragma endregion

#pragma region Pattern state

    /**
     * @brief Checks whether a pattern is currently active.
     *
     * @return true If a pattern is active.
     * @return false Otherwise.
     */
    bool hasActivePattern() const;

    /**
     * @brief Clears the currently active pattern.
     */
    void clearActivePattern();

    /**
     * @brief Returns the currently active pattern.
     *
     * @return const PatternEntry* Active pattern or nullptr.
     */
    const PatternEntry* getActivePattern() const;

#pragma endregion

#pragma region Placed patterns

    /**
     * @brief Removes all placed patterns.
     */
    void clearPlacedPatterns();

    /**
     * @brief Removes the current preview.
     */
    void clearPreview();

    /**
     * @brief Returns all placed patterns.
     *
     * @return const std::vector<PlacedPattern>& Placed patterns.
     */
    [[nodiscard]] const std::vector<PlacedPattern>& getPlacedPatterns() const;

#pragma endregion

    [[nodiscard]] std::string typeName() const override;

  private:
#pragma region Pattern data

    std::vector<PlacedPattern> _placedPatterns;
    int _activePatternIndex = -1;

#pragma endregion

#pragma region References

    dzemikk::Engine* _engine = nullptr;
    game::HexGrid* _grid = nullptr;
    PlayerPatternStatsComponent* _playerPatternStats = nullptr;

#pragma endregion

#pragma region Input listeners

    ListenerID _cancelPatternListenerID = -1;
    ListenerID _rotatePatternListenerID = -1;
    ListenerID _confirmPatternListenerID = -1;

#pragma endregion

#pragma region Preview

    HexCoord _currentPreviewOrigin;
    bool _currentPreviewValid = false;

    dzemikk::GameObject* _previewObject = nullptr;
    std::vector<dzemikk::GameObject*> _previewHexes;
    std::vector<dzemikk::GameObject*> _confirmedHexes;

#pragma endregion

#pragma region Helpers

    void cancelPattern();
    void rebuildPreview();
    glm::vec3 axialToWorld(const HexCoord& coord, float hexSize);
    bool confirmPattern();
    bool isCellOccupiedByPattern(const HexCoord& coord) const;
    void restartPreview();
    void destroyPreview();
    void tryRemovePlacedPatternUnderCursor();
    void removePlacedPattern(size_t index);

#pragma endregion
};

} // namespace game

#endif