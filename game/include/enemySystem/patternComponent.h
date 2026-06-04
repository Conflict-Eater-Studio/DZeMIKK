#pragma once

#include "map/HexPattern.h"

#include <ecs/components/monobehaviour.h>
#include <optional>
#include <vector>

namespace game {

class PatternComponent : public dzemikk::MonoBehaviour {
  public:
    struct PatternEntry {
        HexPattern pattern;
        int count = 0;
    };

    void start() override = 0;
    void update(double deltaTime) override = 0;
    void onDestroy() override = 0;

    void addPattern(const HexPattern& pattern, int count = 1);

    void insertPattern(size_t index, const HexPattern& pattern, int count = 1);

    bool removePattern(size_t index);

    bool removePattern(const HexPattern& pattern);

    void clearPatterns();

    bool canUsePattern(size_t index) const;

    virtual bool usePattern(size_t index) = 0;

    size_t getPatternCount() const;

    PatternEntry* getPattern(size_t index);
    const PatternEntry* getPattern(size_t index) const;

    const std::vector<PatternEntry>& getPatterns() const;

    int findPattern(const HexPattern& pattern) const;

    std::string typeName() const override;

  protected:
    std::vector<PatternEntry> _patterns;
};

} // namespace game