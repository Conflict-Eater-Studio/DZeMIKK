#pragma once

#include "map/HexPattern.h"

#include <cstdint>
#include <ecs/components/monobehaviour.h>
#include <unordered_map>

namespace dzemikk {
class Engine;
}

namespace game {

class PlayerPatternStatsComponent : public dzemikk::MonoBehaviour {
  public:
    struct Stats {
        uint32_t placedTotal = 0;
        uint32_t removedTotal = 0;
        uint32_t actionsTotal = 0;

        std::unordered_map<HexPattern::Type, uint32_t> placedByType;
        std::unordered_map<HexPattern::Type, uint32_t> removedByType;

        float getTypeRatio(HexPattern::Type type) const;
    };

    void start() override {}
    void update(double dt) override;

    void registerPlacement(const HexPattern& pattern);

    void registerRemoval(const HexPattern& pattern);

    const Stats& getStats() const;

    void reset();

    std::string typeName() const override;

    uint32_t getTotalPlaced() const;

    uint32_t getTotalRemoved() const;

    uint32_t getTotalActions() const;

    uint32_t getPlacedCountByType(HexPattern::Type type) const;

    uint32_t getRemovedCountByType(HexPattern::Type type) const;

  private:
    Stats _stats;
};

} // namespace game