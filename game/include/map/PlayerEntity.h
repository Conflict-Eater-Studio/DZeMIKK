#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#include "Entity.h"

namespace game {
    class PlayerEntity : public Entity {
    public:
    PlayerEntity() = default;

    void onEnter(HexCellPtr cell) override;
    void onExit() override;
    void update(double dt) override;

    [[nodiscard]] std::string typeName() const override {
        return "PlayerEntity";
    }

    void tryMove(const HexCellPtr& targetCell);
    void setPath(const std::vector<HexCellPtr>& path);

    private:
        std::vector<HexCellPtr> _path;
        float _duration = 0.0f;
        int _moveCount = 0;
};

inline void PlayerEntity::update(double dt) {
    Entity::update(dt);
    if (_path.empty()) {
        return;
    }

    _duration += dt;

    if (_duration > 0.1f && _moveCount < _path.size()) {
        tryMove(_path[_moveCount % _path.size()]);
        _duration = 0.0f;
        _moveCount++;
    }
}
inline void PlayerEntity::setPath(const std::vector<HexCellPtr>& path) {
    _path = path;
    _moveCount = 1;
}
} // namespace game

#endif // GAME_PLAYER_H
