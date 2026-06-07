#include "stateMachine/combatResolver.h"

#include "player/playerPatternComponent.h"

game::CombatResolver::Result game::CombatResolver::resolve(const PlayerPatternComponent& playerPatterns,
                                     const std::vector<PlannedPattern>& enemyPatterns,
                                     const HexCoord& enemyCenter) {

    std::unordered_map<HexCoord, CellEffect> playerEffects;
    std::unordered_map<HexCoord, CellEffect> enemyEffects;

    for (const auto& placed : playerPatterns.getPlacedPatterns()) {

        const auto& pattern = placed.pattern;

        for (const auto& offset : pattern.getHexes()) {

            auto& effect = playerEffects[offset];

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

            default:
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

            default:
                break;
            }
        }
    }

    Result result;

    std::unordered_set<HexCoord> allOffsets;

    for (const auto& [coord, _] : playerEffects) {
        allOffsets.insert(coord);
    }

    for (const auto& [coord, _] : enemyEffects) {
        allOffsets.insert(coord);
    }

    for (const auto& coord : allOffsets) {

        CellEffect player;
        CellEffect enemy;

        if (auto it = playerEffects.find(coord); it != playerEffects.end()) {
            player = it->second;
        }

        if (auto it = enemyEffects.find(coord); it != enemyEffects.end()) {
            enemy = it->second;
        }

        result.damageToEnemy += std::max(0.0F, player.atk - enemy.def);

        result.damageToPlayer += std::max(0.0F, enemy.atk - player.def);

        result.healToPlayer += player.heal;
        result.healToEnemy += enemy.heal;
    }

    return result;
}
