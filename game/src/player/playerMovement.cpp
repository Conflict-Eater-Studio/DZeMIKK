#include "player/playerMovement.h"

#include "ecs/components/animator.h"
#include "game.h"
#include "gameStateMachine.h"
#include "stateMachine/combatState.h"

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif
#include "animation/animationclip.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"

namespace game {
void PlayerMovement::start() {
    _playerEntity->tryMove(_hexGrid->at({0, 0}));
}
void PlayerMovement::update(double deltaTime) {
    if (_path.empty() || _step >= _path.size()) {
        _animator->setInt("isMoving", 0);
        return;
    }

    _animator->setInt("isMoving", 1);
    HexGrid::HexCellPtr ptr = _path[_step % _path.size()];

    dzemikk::Transform* cellTransform = _world->getHexTransformByCell(*ptr);

    if (cellTransform) {
        if (!_positionCached) {
            _position = cellTransform->getPosition();
            _positionCached = true;
        }

        glm::vec3 newPosition = _position;
        newPosition.y = _playerEntity->getCell()->getHeight();

        cellTransform->setPosition(newPosition);
    }

    if (_animator->getCurrentState()->getClip()->isFinished()) {
        if (cellTransform) {
            cellTransform->setPosition(_position);
            _positionCached = false;
        }
        _playerEntity->tryMove(ptr);
        _animator->setInt("direction", -1);
        _animator->play("Idle");
        _step++;
    }

    if (_step < _path.size()) {
        auto dir = HexCoord::dir(ptr->getCoord() - _playerEntity->getCell()->getCoord());
        if (dir.has_value()) {
#if DZEMIKK_DEV_TOOLS
            spdlog::info("Player moving direction: {}", static_cast<int>(dir.value()));
#endif
            _animator->setInt("direction", static_cast<int>(dir.value()));
            auto* transform = _playerEntity->getOwner()->transform();
            if (transform) {

            }
        }


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
    _positionCached = false;
    _step = 1;
}
void PlayerMovement::setGame(Game* game) {
    _game = game;;
}

void PlayerMovement::stopMovement() {
    _path.clear();
    _step = 0;
    _duration = 0.0;
    _positionCached = false;
}

void PlayerMovement::setAnimator(dzemikk::Animator* animator) {
    _animator = animator;
}

dzemikk::Animator* PlayerMovement::getAnimator() const {
    return _animator;
}
void PlayerMovement::setWorld(World* world) {
    _world = world;
}

} // namespace game