#pragma once

#include <optional>
#include <vector>

#include "map/HexPattern.h"
#include <ecs/components/monobehaviour.h>

namespace dzemikk {
class Engine;
}

namespace game {

class PlayerEntity;
class HexGrid;

class PlayerPatternComponent : public dzemikk::MonoBehaviour {
  public:
    struct PatternEntry {
        HexPattern pattern;
        int count = 0;
    };

    using ListenerID = uint32_t;

    void start() override;
    void update(double deltaTime) override;
    void onDestroy() override;

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

    void setEngine(dzemikk::Engine* engine);

    std::string typeName() const override;

    bool hasActivePattern() const;
    void clearActivePattern();
    const PatternEntry* getActivePattern() const;

  private:
    std::vector<PatternEntry> _patterns;
    int _activePatternIndex = -1;
    dzemikk::Engine* _engine = nullptr;
    ListenerID _cancelPatternListenerID = -1;
    dzemikk::GameObject* _previewObject = nullptr;
    std::vector<dzemikk::GameObject*> _previewHexes;

    void cancelPattern();
    glm::vec3 axialToWorld(const HexCoord& coord, float hexSize);
};

} // namespace game