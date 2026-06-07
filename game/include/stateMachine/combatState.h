#ifndef DZEMIKK_COMBAT_GAMESTATE_H
#define DZEMIKK_COMBAT_GAMESTATE_H

#include "map/HexPattern.h"
#include "stateMachine/combatTypes.h"
#include "stateMachine/iGameState.h"

#include <vector>

class Game;

namespace game {

class PlayerEntity;
class HexChunk;
class HexGrid;
class HexCoord;
class HexCell;
class EnemyEntity;
class PlayerPatternComponent;

/**
 * @brief Handles the turn-based combat gameplay state.
 *
 * Manages combat flow, player and enemy turns, pattern planning,
 * combat resolution, and transitions in and out of combat.
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

    bool _shouldLeaveCombat = false;

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
};

} // namespace game

#endif // DZEMIKK_COMBAT_GAMESTATE_H