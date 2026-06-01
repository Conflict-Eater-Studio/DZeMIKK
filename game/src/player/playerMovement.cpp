#include "player/playerMovement.h"

#include "game.h"
#include "gameStateMachine.h"
#include "stateMachine/combatState.h"

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

} // namespace game