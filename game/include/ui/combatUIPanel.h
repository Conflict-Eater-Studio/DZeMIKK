#pragma once

#include "ecs/components/monobehaviour.h"
#include "enemySystem/patternComponent.h"

#include <assetManager/assetmanager.h>
#include <ecs/components/ui/uiButton.h>
#include <nlohmann/json.hpp>

namespace game {

class CombatUIPanel : public dzemikk::MonoBehaviour {
  public:
    struct PatternUIEntry {
        size_t patternIndex = 0;

        dzemikk::GameObject* root = nullptr;
        dzemikk::UIButton* button = nullptr;
        dzemikk::UITextRenderer* countText = nullptr;
    };

    enum class Mode { AvailablePatterns, EnemyUsage };

  public:
    CombatUIPanel(bool isClickable, Mode mode) : _isClickable(isClickable), _mode(mode) {}

    void start() override;
    void update(double deltaTime) override;

    void refresh();
    void clear();

    void setPatternsComponent(PatternComponent* patterns);
    void setAssetManager(dzemikk::AssetManager* assetManager);
    void setCanvas(dzemikk::GameObject* canvas);
    void refreshVisuals();

  private:
    void buildUI();

    void createPatternSlot(const PatternComponent::PatternEntry& entry, size_t index);

    void createPatternPreview(dzemikk::GameObject* parent, const HexPattern& pattern);

    void refreshCounts();

    dzemikk::GameObject* findPatternsContainer();

    std::string buildPatternName(const HexPattern& pattern);

    glm::vec4 getPatternBaseColor(HexPattern::Type type);

    std::string typeName() const override;
    void setMode(Mode mode);
    int32_t getPatternCount(const PatternComponent::PatternEntry& entry) const;
    glm::vec4 applyUsageTint(glm::vec4 base, uint32_t count);

  private:
    PatternComponent* _patterns = nullptr;

    dzemikk::AssetManager* _assetManager = nullptr;

    dzemikk::GameObject* _canvas = nullptr;
    dzemikk::GameObject* _patternsContainer = nullptr;
    Mode _mode = Mode::AvailablePatterns;

    dzemikk::AssetHandle<nlohmann::json> _patternSlotPrefab;
    dzemikk::AssetHandle<nlohmann::json> _hexUIPrefab;

    std::vector<PatternUIEntry> _uiEntries;
    bool _isClickable = false;

};

} // namespace game