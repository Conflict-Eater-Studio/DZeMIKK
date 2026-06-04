#ifndef GAME_ENEMY_ENTITY_H
#define GAME_ENEMY_ENTITY_H

#include "map/Entity.h"
#include "enemySystem/enemyTypes.h"

namespace game {

class EnemyEntity : public Entity {
  public:
    EnemyEntity() = default;

    [[nodiscard]] std::string typeName() const override {
        return "EnemyEntity";
    }

    void addTerritoryCell(HexCell* cell) {
        if (!cell)
            return;

        _territory.insert(cell);
    }

    const auto& getTerritory() const {
        return _territory;
    }

    void onEnter(HexCellPtr cell) override;
    void onExit() override;

    void setHp(double hp) {
        _hp = hp;
    }

    void setEnemyType(EnemyType type) {
        _type = type;
    }

    void setEnemyPersonality(EnemyPersonality personality) {
        _personality = personality;
    }

    [[nodiscard]] const double getHp() const {
        return _hp;
    }

    [[nodiscard]] const EnemyType getEnemyType() const {
        return _type;
    }

    [[nodiscard]] const EnemyPersonality getEnemyPersonality() const {
        return _personality;
    }

  private:
    std::unordered_set<HexCell*> _territory;
    double _hp = 1;
    EnemyType _type = EnemyType::Normal;
    EnemyPersonality _personality = EnemyPersonality::Balanced;
};

} // namespace game

#endif // GAME_ENEMY_ENTITY_H