#ifndef DZEMIKK_COMBAT_GAMESTATE_H
#define DZEMIKK_COMBAT_GAMESTATE_H

#include "map/HexPattern.h"
#include "stateMachine/combatTypes.h"
#include "stateMachine/iGameState.h"

#include <renderer/material.h>
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
    
    /**
     * @brief Reveals one of the enemy's hidden planned patterns.
     */
    void revealRandomEnemyPattern();

    /**
     * @brief Reveals a single hidden enemy cell.
     */
    void revealRandomEnemyCell();

    /**
     * @brief Removes a cell from the hidden animation set.
     *
     * @param cell Cell to remove.
     */
    void removeHiddenHex(HexCell* cell);
    [[nodiscard]] EnemyEntity* getCurrentEnemy() const;


  private:
    Game* _game = nullptr;
    PlayerEntity* _player = nullptr;
    EnemyEntity* _currentEnemy = nullptr;
    PlayerPatternComponent* _playerPatternComponent = nullptr;

    CombatPhase _phase = CombatPhase::PreparingBoard;

    ListenerID _endTurnListenerId = -1;

    std::vector<PlannedPattern> _plannedPatterns;
    
    /**
     * @brief Enemy patterns that have already been revealed.
     */
    std::unordered_set<int> _revealedPatterns;

    /**
     * @brief Enemy cells that have already been revealed.
     */
    std::unordered_set<HexCell*> _revealedCells;

    /**
     * @brief Center cell of the combat arena.
     */
    HexCell* _arenaCenterCell = nullptr;

    bool _shouldLeaveCombat = false;

    bool _playerDied = false;

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
    float _hiddenOffsetY = -8.0F;

    /**
     * @brief Speed of the board enter animation.
     */
    float _animationEnterSpeed = 0.8F;

    /**
     * @brief Speed of the board exit animation.
     */
    float _animationExitSpeed = 1.0F;

    /**
     * @brief Number of completed combat rounds.
     */
    int _roundCount = 0;

    /**
     * @brief Timer used to delay result processing and transitions.
     */
    float _resultTimer = 0;

    /**
     * @brief Stores animation data for a single combat cell.
     */
    struct AnimatedHex {
        dzemikk::Transform* transform;
        float startY;
        int distance;
    };

    struct PlannedHexAnimation {
        dzemikk::Transform* transform;
        float startY;
    };

    std::vector<AnimatedHex> _hiddenHexes;

    std::vector<PlannedHexAnimation> _plannedHexAnimations;

    bool _enemyPlanAnimation = false;
    float _enemyPlanAnimationTime = 0.0f;

    float _enemyPlanDuration = 0.5f;
    float _enemyPlanHeight = 2.0f;

    bool _enemyPlanDelay = false;
    float _enemyPlanDelayTimer = 0.0f;

    std::unordered_map<HexPattern::Type, std::shared_ptr<dzemikk::Material>> _hexMaterials;
    std::shared_ptr<dzemikk::Material> _enemyBattleHexMaterial;
    std::shared_ptr<dzemikk::Material> _emptyEnemyBattleHexMaterial;
    std::shared_ptr<dzemikk::Material> _showedPatternMaterial;

    float _playerTurnTextTimer = 0.0f;

    /**
     * @brief Starts a new combat turn.
     */
    void startNewTurn();

    /**
     * @brief Ends the player's turn and advances combat flow.
     */
    void endPlayerTurn();

    std::shared_ptr<dzemikk::Material> getPatternMaterial(HexPattern::Type type);

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

    /**
     * @brief Reveals a planned pattern by index.
     *
     * @param index Index of the pattern to reveal.
     */
    void showPattern(int index);

    /**
     * @brief Displays a pattern color on a specific cell.
     *
     * @param cell Target cell.
     * @param type Pattern type used to determine the color.
     */
    void showCellColor(HexCell* cell, HexPattern::Type type);
};

} // namespace game

#endif // DZEMIKK_COMBAT_GAMESTATE_H