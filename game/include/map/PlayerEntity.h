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
};
inline void PlayerEntity::update(double dt) {
    Entity::update(dt);
    if (_path.empty()) {
        return;
    }
    if (_path.front() == getCell()) {
        _path.erase(_path.begin());
    }

    _duration += dt;

    if (_duration > 0.5f) {
        tryMove(_path.front());
        _duration = 0.0f;
    }
}
inline void PlayerEntity::setPath(const std::vector<HexCellPtr>& path) {
    _path = path;
}
} // namespace game

#endif // GAME_PLAYER_H
