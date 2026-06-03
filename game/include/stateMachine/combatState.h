#ifndef DZEMIKK_COMBAT_GAMESTATE_H
#define DZEMIKK_COMBAT_GAMESTATE_H

#include "stateMachine/iGameState.h"
#include <vector>

#include "map/HexPattern.h"

class Game;

namespace game {
class PlayerEntity;
class HexChunk;
class HexGrid;
class HexCoord;
class HexCell;
class EnemyEntity;
class PlayerPatternComponent;

class CombatState : public IGameState {
  public:
    using ListenerID = uint32_t;

    enum class CombatPhase { PreparingBoard, EnemyPlanning, PlayerTurn, ResolveTurn };

    struct PlacementCandidate {
        HexPattern* pattern = nullptr;
        std::vector<HexCell*> cells;
        float score = 0.0f;
    };

    struct PlannedPattern {
        HexPattern::Type type;
        float strength = 0.0f;
        std::vector<HexCell*> cells;
    };

    CombatState(Game* game) : _game(game) {}

    void onEnter() override;

    void onExit() override;

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

    void startNewTurn();
    void endPlayerTurn();
    void generateEnemyBlockedCells();
    float getTypeWeight(const EnemyEntity* enemy, HexPattern::Type type);
    float scorePattern(const EnemyEntity* enemy, const HexPattern& pattern);
    std::vector<PlacementCandidate> generateCandidates(const std::vector<HexCell*>& availableCells);

    bool tryPlacePattern(HexCell* anchor, const HexPattern& pattern,
                         std::vector<HexCell*>& outCells);

    std::optional<PlacementCandidate> chooseCandidate(std::vector<PlacementCandidate>& candidates);

    void fillEnemyBoard(float coverage = 0.33f);
    static glm::vec4 getPatternColor(HexPattern::Type type);
    void showEnemyPlannedPatterns();
    HexPattern::Type choosePatternType() const;
    void resolveConflict();
};

} // namespace game
#endif