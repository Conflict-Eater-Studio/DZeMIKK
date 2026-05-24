#include "playerMovement.h"

namespace game {

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

    if (_duration > 0.1f && _step < _path.size()) {
        HexGrid::HexCellPtr ptr = _path[_step % _path.size()];
        _playerEntity->tryMove(ptr);
        _duration = 0.0f;
        _step++;
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
} // namespace game