#ifndef DZEMIKK_PLAYERMOVEMENT_H
#define DZEMIKK_PLAYERMOVEMENT_H

#include "../map/HexGrid.h"
#include "../map/PlayerEntity.h"
#include "ecs/components/monobehaviour.h"

class Game;
namespace dzemikk {
    class Animator;
}
namespace game {

class PlayerPatternComponent;

class PlayerMovement : public dzemikk::MonoBehaviour {
  public:
    using Base = dzemikk::MonoBehaviour;

    PlayerMovement() = default;
    using LerpCallback = std::function<void(float progress)>;
    struct CellLerp {
        HexGrid::HexCellPtr cell;
        float startY;
        float targetY;
        float progress;
        LerpCallback onComplete;
    };
    class World;
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
        void setGame(Game* game);
        void stopMovement();

        void setAnimator(dzemikk::Animator* animator);
        [[nodiscard]] dzemikk::Animator* getAnimator() const;

        void setWorld(World* world);
    private:
        void lerpCellTo(const HexGrid::HexCellPtr& cell, float targetY, LerpCallback callback);
        void updateCellLerps(double deltaTime);
        void rotateToDirection(int direction);
        bool hasAllLerpsFinished();
        float directionToAngle(int direction);

        std::vector<CellLerp> _cellLerps;
        std::vector<HexGrid::HexCellPtr> _path;
        std::vector<HexGrid::HexCellPtr> _cachedPath;

        World* _world = nullptr;
        PlayerEntity* _playerEntity = nullptr;
        HexGrid* _hexGrid = nullptr;

        Game* _game = nullptr;
        dzemikk::Animator* _animator = nullptr;
        glm::vec3 _position;

    static bool isWalkableCell(const HexGrid::HexCellPtr& cell);
    void tryUnlockBlockingPattern(const HexGrid::HexCellPtr& cell);
};
} // namespace game
        float _lerpSpeed = 0.2f;
        int _playerDir = 3;
        int _step = 1;
        bool isFallingFinished = true;
        float _moveTimer = 0.0f;
        float _moveDelay = 0.5f;
    };
}

#endif
