#pragma once

#include <optional>
#include <vector>

#include "map/HexPattern.h"
#include <ecs/components/monobehaviour.h>

namespace game {

class PlayerEntity;
class HexGrid;

class PlayerPatternComponent : public dzemikk::MonoBehaviour {
  public:
    struct PatternEntry {
        HexPattern pattern;
        int count = 0;
    };

    void start() override;
    void update(double deltaTime) override;

    void addPattern(const HexPattern& pattern, int count = 1);

    void insertPattern(size_t index, const HexPattern& pattern, int count = 1);


    bool removePattern(size_t index);

    bool removePattern(const HexPattern& pattern);

    void clearPatterns();


    bool addCount(size_t index, int amount);

    bool removeCount(size_t index, int amount);

    bool setCount(size_t index, int count);


    bool canUsePattern(size_t index) const;

    bool usePattern(size_t index);


    size_t getPatternCount() const;

    PatternEntry* getPattern(size_t index);
    const PatternEntry* getPattern(size_t index) const;

    const std::vector<PatternEntry>& getPatterns() const;


    int findPattern(const HexPattern& pattern) const;

  private:
    std::vector<PatternEntry> _patterns;

    // Odziedziczono za poœrednictwem elementu MonoBehaviour
    std::string typeName() const override;
};

} // namespace game