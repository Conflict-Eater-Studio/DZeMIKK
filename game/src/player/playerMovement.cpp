#include "player/playerMovement.h"

#include <algorithm>
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
    if (!_animator) return;

    updateCellLerps(deltaTime);

    if (_path.empty() || _step >= _path.size()) {
        _animator->setInt("isMoving", 0);
        return;
    }

    _moveTimer += deltaTime;
    HexGrid::HexCellPtr currentTargetCell = _path[_step % _path.size()];
    _animator->setInt("isMoving", 1);

    if (isFallingFinished) {
        lerpCellTo(currentTargetCell, _playerEntity->getCell()->getHeight(), [this](float progress) {
            isFallingFinished = false;
        });
    }

    dzemikk::Transform* cellTransform = _world->getHexTransformByCell(*currentTargetCell);

    if (_animator->getCurrentState()->getClip()->isFinished()) {
        if (cellTransform) {
            lerpCellTo(currentTargetCell, currentTargetCell->getHeight(), [this](float progress) {
                isFallingFinished = true;
            });

            if (!_cachedPath.empty()) {
                _path = _cachedPath;
                _cachedPath.clear();
                _step = 1;
            }
        }
        rotateToDirection(_playerDir);
        _playerEntity->tryMove(currentTargetCell);
        _animator->setInt("direction", -1);
        _animator->play("Idle");
        _step++;
    }


    if (_step < _path.size() && _moveTimer >= _moveDelay) {
        auto dir = HexCoord::dir(currentTargetCell->getCoord() - _playerEntity->getCell()->getCoord());
        if (dir.has_value()) {
            int hexDir = static_cast<int>(dir.value());
            int relativeDir = 3;
            if (hexDir == _playerDir) {
                _animator->setInt("direction",relativeDir);
            }else {
                int offset = hexDir - _playerDir;
                int anim = (relativeDir + offset + 12) % 12;
                _animator->setInt("direction",  anim);
            }

            _playerDir = hexDir;
            _moveTimer = 0.0f;
        }
    }

    if (auto cell = _playerEntity->getCell()) {
        if (cell->getType() == HexCell::Type::EnemyBattleHex && _game) {
            _game->getStateMachine()->setState(std::make_unique<game::CombatState>(_game));
        }
    }
}
void PlayerMovement::setSpeed(float speed) {
    _lerpSpeed = speed;
}
float PlayerMovement::getSpeed() const {
    return _lerpSpeed;
}
void PlayerMovement::setPlayerEntity(PlayerEntity* playerEntity) {
    _playerEntity = playerEntity;
}
void PlayerMovement::setHexGrid(HexGrid* hexGrid) {
    _hexGrid = hexGrid;
}
void PlayerMovement::moveTo(HexGrid::HexCellPtr cell) {
    std::vector<HexGrid::HexCellPtr> path = _hexGrid->findPath(_playerEntity->getCell(), cell);
    if (!_animator->getCurrentState()->getClip()->isFinished()){
        _cachedPath = path;
    }
    if (_animator->getCurrentState()->getName() == "Idle") {
        _path = path;
        _step = 1;
    };
}
void PlayerMovement::setGame(Game* game) {
    _game = game;
}

void PlayerMovement::stopMovement() {
    _path.clear();
    _step = 0;
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
void PlayerMovement::lerpCellTo(const HexGrid::HexCellPtr& cell, float targetY, LerpCallback callback)  {
    for (auto& lerp : _cellLerps) {
        if (lerp.cell == cell) {
            if (lerp.targetY == targetY) {
                return;
            }
            float t = std::min(lerp.progress, 1.0f);
            lerp.targetY = targetY;
            lerp.progress = t;
            lerp.onComplete = callback;
            return;
        }
    }

    dzemikk::Transform* cellTransform = _world->getHexTransformByCell(*cell);
    if (!cellTransform) {
        return;
    }
    _cellLerps.push_back({cell, cellTransform->getPosition().y, targetY, 0.0f, callback});
}

void PlayerMovement::updateCellLerps(double deltaTime) {
    for (auto it = _cellLerps.begin(); it != _cellLerps.end();) {
        it->progress += _lerpSpeed;

        float t = std::min(it->progress, 1.0f);

        dzemikk::Transform* cellTransform = _world->getHexTransformByCell(*it->cell);
        if (cellTransform) {
            glm::vec3 pos = cellTransform->getPosition();
            pos.y = it->startY + (it->targetY - it->startY) * t;
            cellTransform->setPosition(pos);
        }
        if (it->progress >= 1.0f) {
            LerpCallback callback = it->onComplete;
            it = _cellLerps.erase(it);
            callback(1.0f);
        } else {
            ++it;
        }
    }
}
void PlayerMovement::rotateToDirection(int direction) {
    float angle = directionToAngle(direction);
    _playerEntity->getOwner()->transform()->setRotation(
        glm::angleAxis(glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)));
}

float PlayerMovement::directionToAngle(int direction) {
    float angle = -direction * 30.0f;
    return angle;
}
} // namespace game