#include "enemySystem/territoryPatternRegistry.h"

game::TerritoryPatternRegistry& game::TerritoryPatternRegistry::instance() {
    static TerritoryPatternRegistry inst;
    return inst;
}

void game::TerritoryPatternRegistry::registerPattern(const TerritoryPattern& pattern) {
    _patterns[pattern.name] = pattern;
}

const game::TerritoryPattern* game::TerritoryPatternRegistry::get(const std::string& name) const {
    auto it = _patterns.find(name);

    if (it == _patterns.end()) {
        return nullptr;
    }

    return &it->second;
}