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

        void setGame(Game* game);
        void stopMovement();

        void setAnimator(dzemikk::Animator* animator);
        [[nodiscard]] dzemikk::Animator* getAnimator() const;

        void setWorld(World* world);
    private:
        PlayerEntity* _playerEntity = nullptr;
        HexGrid* _hexGrid = nullptr;
        std::vector<HexGrid::HexCellPtr> _path;
        std::vector<HexGrid::HexCellPtr> _cachedPath;

        float _speed = 1.0f;
        int _step = 1;
        float _duration = 0.0f;
        glm::vec3 _position;
        bool _positionCached = false;
        Game* _game = nullptr;
        dzemikk::Animator* _animator = nullptr;
        World* _world = nullptr;

    };
}

#endif
