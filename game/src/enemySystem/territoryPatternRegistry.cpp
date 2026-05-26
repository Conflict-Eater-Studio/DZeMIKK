#include "enemySystem/territoryPatternRegistry.h"

namespace game {

TerritoryPatternRegistry& TerritoryPatternRegistry::instance() {
    static TerritoryPatternRegistry inst;
    return inst;
}

void TerritoryPatternRegistry::registerPattern(const TerritoryPattern& pattern) {
    _patterns[pattern.name] = pattern;
}

const TerritoryPattern* TerritoryPatternRegistry::get(const std::string& name) const {
    auto it = _patterns.find(name);

    if (it == _patterns.end())
        return nullptr;

    return &it->second;
}

} // namespace game