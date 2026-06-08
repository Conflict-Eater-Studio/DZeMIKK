#include "stateMachine/combatResolver.h"

#include "player/playerPatternComponent.h"

game::CombatResolver::Result
game::CombatResolver::resolve(const PlayerPatternComponent& playerPatterns,
                              const std::vector<PlannedPattern>& enemyPatterns,
                              const HexCoord& enemyCenter, const HexCoord& playerCenter) {

    std::unordered_map<HexCoord, CellEffect> playerEffects;
    std::unordered_map<HexCoord, CellEffect> enemyEffects;

    for (const auto& placed : playerPatterns.getPlacedPatterns()) {
        const auto& pattern = placed.pattern;

        for (const auto& cell : placed.worldCells) {

            HexCoord coord = playerCenter - cell;
            auto& effect = playerEffects[coord];

            switch (pattern.getType()) {
            case HexPattern::Type::ATK:
                effect.atk += pattern.getEffectStrength();
                break;
            case HexPattern::Type::DEF:
                effect.def += pattern.getEffectStrength();
                break;
            case HexPattern::Type::HEAL:
                effect.heal += pattern.getEffectStrength();
                break;
            }
        }
    }

    for (const auto& pattern : enemyPatterns) {

        for (auto* cell : pattern.cells) {

            if (!cell) {
                continue;
            }

            HexCoord offset = enemyCenter - cell->getCoord();

            auto& effect = enemyEffects[offset];

            switch (pattern.type) {
            case HexPattern::Type::ATK:
                effect.atk += pattern.strength;
                break;
            case HexPattern::Type::DEF:
                effect.def += pattern.strength;
                break;
            case HexPattern::Type::HEAL:
                effect.heal += pattern.strength;
                break;
            }
        }
    }

    // --- ALL OFFSETS ---
    std::unordered_set<HexCoord> allOffsets;

    for (const auto& [coord, _] : playerEffects)
        allOffsets.insert(coord);
    for (const auto& [coord, _] : enemyEffects)
        allOffsets.insert(coord);

    Result result;

    for (const auto& coord : allOffsets) {

        CellEffect player;
        CellEffect enemy;

        if (auto it = playerEffects.find(coord); it != playerEffects.end())
            player = it->second;

        if (auto it = enemyEffects.find(coord); it != enemyEffects.end())
            enemy = it->second;

        float dmgEnemy = std::max(0.0F, player.atk - enemy.def);
        float dmgPlayer = std::max(0.0F, enemy.atk - player.def);

        result.damageToEnemy += dmgEnemy;
        result.damageToPlayer += dmgPlayer;
        result.healToPlayer += player.heal;
        result.healToEnemy += enemy.heal;
    }

    return result;
}
