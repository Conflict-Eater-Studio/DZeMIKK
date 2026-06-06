#ifndef GAME_ENEMY_ENTITY_H
#define GAME_ENEMY_ENTITY_H

#include "enemySystem/enemyActionWeights.h"
#include "enemySystem/enemyTypes.h"
#include "map/Entity.h"

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

    void onEnter(HexCellPtr cell);

    void onExit() override;

    void setHp(double hp) {
        _hp = hp;
    }

    void setConfig(const EnemySpawnConfig& config) {
        _config = config;
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

    void setActionWeights(const EnemyActionWeights& weights) {
        _actionWeights = weights;
        _actionWeights.normalize();
    }

    const EnemyActionWeights& getActionWeights() const {
        return _actionWeights;
    }

    void addBlockedCell(HexCell* cell) {
        if (!cell)
            return;

        _blockedEnemyCells.insert(cell);
    }

    const auto& getBlockedCells() const {
        return _blockedEnemyCells;
    }

    void clearBlockedCells() {
        _blockedEnemyCells.clear();
    }

    bool isCellBlocked(HexCell* cell) const {
        return cell && _blockedEnemyCells.contains(cell);
    }

    [[nodiscard]] EnemySpawnConfig getConfig() const {
        return _config;
    }

  private:
    std::unordered_set<HexCell*> _territory;
    std::unordered_set<HexCell*> _blockedEnemyCells;
    double _hp = 1;
    EnemyType _type = EnemyType::Normal;
    EnemyPersonality _personality = EnemyPersonality::Balanced;
    EnemyActionWeights _actionWeights;
    EnemySpawnConfig _config;
};

} // namespace game

#endif // GAME_ENEMY_ENTITY_H
