#include "player/playerMovement.h"

#include "game.h"
#include "gameStateMachine.h"
#include "stateMachine/combatState.h"

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"

namespace game {
//TO DO:  While finding a path, the player should avoid occupied fields and opponents' territories

void PlayerMovement::start() {
    MonoBehaviour::start();
    _playerEntity->tryMove(_hexGrid->at({0, 0}));
}
void PlayerMovement::update(double deltaTime) {
    MonoBehaviour::update(deltaTime);
    if (_path.empty() || _step >= _path.size()) {
        return;
    }

    _duration += deltaTime;

    if (_duration > _speed && _step < _path.size()) {
        HexGrid::HexCellPtr ptr = _path[_step % _path.size()];

        auto dir = HexCoord::dir(ptr->getCoord() - _playerEntity->getCell()->getCoord());
        if (dir.has_value()) {
#if DZEMIKK_DEV_TOOLS
            spdlog::info("Player moving direction: {}", static_cast<int>(dir.value()));
#endif
            auto* transform = _playerEntity->getOwner()->transform();
            if (transform) {
                float yaw = 0.0f;
                switch (*dir) {
                    case HexCoord::Direction::R0:   yaw = 0.0f;   break;
                    case HexCoord::Direction::R30:  yaw = -30.0f;  break;
                    case HexCoord::Direction::R60:  yaw = -60.0f;  break;
                    case HexCoord::Direction::R90:  yaw = -90.0f;  break;
                    case HexCoord::Direction::R120: yaw = -120.0f; break;
                    case HexCoord::Direction::R150: yaw = -150.0f; break;
                    case HexCoord::Direction::R180: yaw = -180.0f; break;
                    case HexCoord::Direction::R210: yaw = -210.0f; break;
                    case HexCoord::Direction::R240: yaw = -240.0f; break;
                    case HexCoord::Direction::R270: yaw = -270.0f; break;
                    case HexCoord::Direction::R300: yaw = -300.0f; break;
                    case HexCoord::Direction::R330: yaw = -330.0f; break;
                }
                transform->setEulerAngles({0.0f, yaw, 0.0f});
            }
        }

        _playerEntity->tryMove(ptr);
        _duration = 0.0f;
        _step++;
    }

    if (auto cell = _playerEntity->getCell()) {
        if (cell->getType() == HexCell::Type::EnemyBattleHex && _game) {
            _game->getStateMachine()->setState(std::make_unique<game::CombatState>(_game));
        }
    }
}
void PlayerMovement::setSpeed(float speed) {
    _speed = speed;
}
float PlayerMovement::getSpeed() const {
    return _speed;
}
void PlayerMovement::setPlayerEntity(PlayerEntity* playerEntity) {
    _playerEntity = playerEntity;
}
void PlayerMovement::setHexGrid(HexGrid* hexGrid) {
    _hexGrid = hexGrid;
}
void PlayerMovement::moveTo(HexGrid::HexCellPtr cell) {
    _path = _hexGrid->findPath(_playerEntity->getCell(), cell);
    _step = 1;
}
void PlayerMovement::setGame(Game* game) {
    _game = game;
}

void PlayerMovement::stopMovement() {
    _path.clear();
    _step = 0;
    _duration = 0.0;
}

void PlayerMovement::setAnimator(dzemikk::Animator* animator) {
    _animator = animator;
}

dzemikk::Animator* PlayerMovement::getAnimator() const {
    return _animator;
}

} // namespace game