#pragma once

#include <optional>
#include <vector>

#include "map/HexPattern.h"
#include "enemySystem/patternComponent.h"

namespace dzemikk {
class Engine;
}

namespace game {

class PlayerEntity;
class HexGrid;
class PlayerPatternStatsComponent;

class PlayerPatternComponent : public PatternComponent {
  public:

    struct PlacedPattern {
        HexPattern pattern;
        HexCoord origin;
        std::vector<dzemikk::GameObject*> objects;
    };

    using ListenerID = uint32_t;

    void start() override;
    void update(double deltaTime) override;
    void onDestroy() override;

    bool addCount(size_t index, int amount);

    bool removeCount(size_t index, int amount);

    bool setCount(size_t index, int count);

    bool usePattern(size_t index) override;

    void setEngine(dzemikk::Engine* engine);

    std::string typeName() const override;

    bool hasActivePattern() const;
    void clearActivePattern();
    const PatternEntry* getActivePattern() const;

    void setGrid(game::HexGrid* grid);
    void clearPlacedPatterns();
    void clearPreview();

    const std::vector<PlacedPattern>& getPlacedPatterns() const;

  private:
    std::vector<PlacedPattern> _placedPatterns;
    int _activePatternIndex = -1;
    dzemikk::Engine* _engine = nullptr;
    ListenerID _cancelPatternListenerID = -1;
    ListenerID _rotatePatternListenerID = -1;
    ListenerID _confirmPatternListenerID = -1;
    HexCoord _currentPreviewOrigin;
    bool _currentPreviewValid = false;

    dzemikk::GameObject* _previewObject = nullptr;
    std::vector<dzemikk::GameObject*> _previewHexes;
    std::vector<dzemikk::GameObject*> _confirmedHexes;
    game::HexGrid* _grid = nullptr;

    PlayerPatternStatsComponent* _playerPatternStats = nullptr;

    void cancelPattern();
    void rebuildPreview();
    glm::vec3 axialToWorld(const HexCoord& coord, float hexSize);
    bool confirmPattern();
    bool isCellOccupiedByPattern(const HexCoord& coord) const;
    void restartPreview();
    void destroyPreview();
    void tryRemovePlacedPatternUnderCursor();
    void removePlacedPattern(size_t index);
};

} // namespace game