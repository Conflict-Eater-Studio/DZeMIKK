#pragma once

#include "enemySystem/territoryPattern.h"

#include <unordered_map>

namespace game {

class TerritoryPatternRegistry {
  public:
    static TerritoryPatternRegistry& instance();

    void registerPattern(const TerritoryPattern& pattern);

    const TerritoryPattern* get(const std::string& name) const;

  private:
    std::unordered_map<std::string, TerritoryPattern> _patterns;
};

} // namespace game