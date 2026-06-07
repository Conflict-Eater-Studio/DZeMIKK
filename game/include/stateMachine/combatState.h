#ifndef DZEMIKK_COMBAT_GAMESTATE_H
#define DZEMIKK_COMBAT_GAMESTATE_H

#include "map/HexPattern.h"
#include "stateMachine/combatTypes.h"
#include "stateMachine/iGameState.h"

#include <vector>

class Game;

namespace dzemikk {
class Transform;
}

namespace game {

class PlayerEntity;
class HexChunk;
class HexGrid;
class HexCoord;
class HexCell;
class EnemyEntity;
class PlayerPatternComponent;
class World;

/**
 * @brief Handles the turn-based combat gameplay state.
 *
 * Manages combat flow, player and enemy turns, pattern planning,
 * combat resolution, combat board animations, and transitions in and out
 * of combat.
 */
class CombatState : public IGameState {
  public:
    using ListenerID = uint32_t;

    /**
     * @brief Constructs a new CombatState instance.
     *
     * @param game Owning game instance.
     */
    CombatState(Game* game) : _game(game) {}

    /**
     * @brief Called when entering the combat state.
     */
    void onEnter() override;

    /**
     * @brief Called when leaving the combat state.
     */
    void onExit() override;

    /**
     * @brief Updates the combat state.
     *
     * @param dt Time elapsed since the previous frame.
     */
    void onUpdate(float dt) override;

  private:
    Game* _game = nullptr;
    PlayerEntity* _player = nullptr;
    EnemyEntity* _currentEnemy = nullptr;
    PlayerPatternComponent* _playerPatternComponent = nullptr;

    CombatPhase _phase = CombatPhase::PreparingBoard;

    ListenerID _endTurnListenerId = -1;

    std::vector<PlannedPattern> _plannedPatterns;

    /**
     * @brief Center cell of the combat arena.
     */
    HexCell* _arenaCenterCell = nullptr;

    bool _shouldLeaveCombat = false;

    /**
     * @brief Current progress of the board visibility animation.
     */
    float _boardTransition = 0.0F;

    /**
     * @brief Indicates whether the board enter animation is active.
     */
    bool _enterAnimation = true;

    /**
     * @brief Indicates whether the board exit animation is active.
     */
    bool _exitAnimation = false;

    /**
     * @brief Vertical offset applied to hidden cells.
     */
    float _hiddenOffsetY = -5.0F;

    /**
     * @brief Speed of the board enter animation.
     */
    float _animationEnterSpeed = 0.5F;

    /**
     * @brief Speed of the board exit animation.
     */
    float _animationExitSpeed = 1.0F;

    int _roundCount = 0;

    /**
     * @brief Stores animation data for a single combat cell.
     */
    struct AnimatedHex {
        dzemikk::Transform* transform;
        float startY;
        int distance;
    };

    std::vector<AnimatedHex> _hiddenHexes;

    /**
     * @brief Starts a new combat turn.
     */
    void startNewTurn();

    /**
     * @brief Ends the player's turn and advances combat flow.
     */
    void endPlayerTurn();

    /**
     * @brief Returns the display color associated with a pattern type.
     *
     * @param type Pattern type.
     *
     * @return glm::vec4 Color used for visualization.
     */
    static glm::vec4 getPatternColor(HexPattern::Type type);

    /**
     * @brief Displays the enemy's planned pattern placements.
     */
    void showEnemyPlannedPatterns();

    /**
     * @brief Resolves the current turn's combat interactions.
     */
    void resolveConflict();

    /**
     * @brief Updates combat board visibility animation.
     *
     * @param factor Normalized animation progress.
     * @param exiting True when playing the exit animation.
     */
    void updateBoardVisibility(float factor, bool exiting);

    /**
     * @brief Initializes combat-specific state and board setup.
     */
    void initializeCombat();

    /**
     * @brief Registers combat input handlers.
     */
    void setupInput();

    /**
     * @brief Creates and configures the enemy health display.
     */
    void setupEnemyHealth();

    /**
     * @brief Collects cells that participate in board animations.
     */
    void collectAnimatedHexes();

    /**
     * @brief Determines whether a cell should be animated.
     *
     * @param cell Cell being evaluated.
     * @param centerCoord Arena center coordinate.
     * @param visibleRadius Visible combat radius.
     * @param playerCell Player position cell.
     * @param enemyCell Enemy position cell.
     *
     * @return true If the cell should be animated.
     * @return false Otherwise.
     */
    bool shouldAnimateCell(HexCell* cell, const HexCoord& centerCoord, int visibleRadius,
                           HexCell* playerCell, HexCell* enemyCell) const;

    /**
     * @brief Adds a cell to the animation list.
     *
     * @param cell Cell to animate.
     * @param world World containing the cell entity.
     * @param centerCoord Arena center coordinate.
     */
    void addCellToAnimation(HexCell* cell, World* world, const HexCoord& centerCoord);
};

} // namespace game

#endif // DZEMIKK_COMBAT_GAMESTATE_H