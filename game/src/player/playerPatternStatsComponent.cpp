#include "player/playerPatternStatsComponent.h"
#include <iostream>

float game::PlayerPatternStatsComponent::Stats::getTypeRatio(HexPattern::Type type) const {
    if (actionsTotal == 0) {
        return 0.0f;
    }

    auto it = placedByType.find(type);
    uint32_t value = (it != placedByType.end()) ? it->second : 0;
    return static_cast<float>(value) / static_cast<float>(actionsTotal);
}

void game::PlayerPatternStatsComponent::update(double dt) {
    /*
    static double printTimer = 0.0;
    printTimer += dt;

    if (printTimer < 1.0)
        return;

    printTimer = 0.0;

    std::cout << "\n========== PATTERN STATS ==========\n";
    std::cout << "Placed total  : " << _stats.placedTotal << '\n';
    std::cout << "Removed total : " << _stats.removedTotal << '\n';
    std::cout << "Actions total : " << _stats.actionsTotal << '\n';

    std::cout << "\nPlaced by type:\n";
    for (const auto& [type, count] : _stats.placedByType) {
        std::cout << "  Type " << static_cast<int>(type) << " -> " << count << '\n';
    }

    std::cout << "\nRemoved by type:\n";
    for (const auto& [type, count] : _stats.removedByType) {
        std::cout << "  Type " << static_cast<int>(type) << " -> " << count << '\n';
    }

    std::cout << "===================================\n";
    */
}

void game::PlayerPatternStatsComponent::registerPlacement(const HexPattern& pattern) {
    _stats.placedTotal++;
    _stats.actionsTotal++;
    _stats.placedByType[pattern.getType()]++;
}

void game::PlayerPatternStatsComponent::registerRemoval(const HexPattern& pattern) {
    _stats.removedTotal++;
    _stats.actionsTotal++;
    _stats.removedByType[pattern.getType()]++;
}

const game::PlayerPatternStatsComponent::Stats&
game::PlayerPatternStatsComponent::getStats() const {
    return _stats;
}

std::string game::PlayerPatternStatsComponent::typeName() const {
    return "PlayerPatternStatsComponent";
}

void game::PlayerPatternStatsComponent::reset() {
    _stats = Stats{};
}

uint32_t game::PlayerPatternStatsComponent::getTotalPlaced() const {
    return _stats.placedTotal;
}

uint32_t game::PlayerPatternStatsComponent::getTotalRemoved() const {
    return _stats.removedTotal;
}

uint32_t game::PlayerPatternStatsComponent::getTotalActions() const {
    return _stats.actionsTotal;
}

uint32_t game::PlayerPatternStatsComponent::getPlacedCountByType(HexPattern::Type type) const {
    auto it = _stats.placedByType.find(type);
    if (it == _stats.placedByType.end()) {
        return 0;
    }

    return it->second;
}

uint32_t game::PlayerPatternStatsComponent::getRemovedCountByType(HexPattern::Type type) const {
    auto it = _stats.removedByType.find(type);
    if (it == _stats.removedByType.end()) {
        return 0;
    }

    return it->second;
}
