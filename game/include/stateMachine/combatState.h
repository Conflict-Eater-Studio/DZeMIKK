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
        HexPattern pattern;
        std::vector<HexCell*> cells;
        float score = 0.0f;
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

    void startNewTurn();
    void endPlayerTurn();
    void generateEnemyBlockedCells();
    float getTypeWeight(const EnemyEntity* enemy, HexPattern::Type type);
    float scorePattern(const EnemyEntity* enemy, const HexPattern& pattern);
};

} // namespace game
#endif