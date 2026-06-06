#ifndef DZEMIKK_PLAYERMOVEMENT_H
#define DZEMIKK_PLAYERMOVEMENT_H

#include "../map/HexGrid.h"
#include "../map/PlayerEntity.h"
#include "ecs/components/monobehaviour.h"

class Game;
namespace game {

class PlayerPatternComponent;

class PlayerMovement : public dzemikk::MonoBehaviour {
  public:
    PlayerMovement() = default;

    void start() override;
    void update(double deltaTime) override;
    [[nodiscard]] std::string typeName() const override {
        return "PlayerMovement";
    }

    void setSpeed(float speed);
    float getSpeed() const;
    void setPlayerEntity(PlayerEntity* playerEntity);
    void setHexGrid(HexGrid* hexGrid);
    void moveTo(HexGrid::HexCellPtr cell);

    void setGame(Game* game);
    void stopMovement();

    [[nodiscard]] std::vector<HexGrid::HexCellPtr> findPath(const HexGrid::HexCellPtr& startCell,
                                                            const HexGrid::HexCellPtr& targetCell);

  private:
    PlayerEntity* _playerEntity = nullptr;
    HexGrid* _hexGrid = nullptr;
    std::vector<HexGrid::HexCellPtr> _path;
    float _speed = 1.0F;
    int _step = 1;
    float _duration = 0.0F;

    Game* _game = nullptr;

    static bool isWalkableCell(const HexGrid::HexCellPtr& cell);
    void tryUnlockBlockingPattern(const HexGrid::HexCellPtr& cell);
};
} // namespace game

#endif
