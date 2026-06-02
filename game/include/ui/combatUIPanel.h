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

  public:
    CombatUIPanel(bool isClickable) : _isClickable(isClickable) {}

    void start() override;
    void update(double deltaTime) override;

    void refresh();
    void clear();

    void setPatternsComponent(PatternComponent* patterns);
    void setAssetManager(dzemikk::AssetManager* assetManager);
    void setCanvas(dzemikk::GameObject* canvas);

  private:
    void buildUI();

    void createPatternSlot(const PatternComponent::PatternEntry& entry, size_t index);

    void createPatternPreview(dzemikk::GameObject* parent, const HexPattern& pattern);

    void refreshCounts();

    dzemikk::GameObject* findPatternsContainer();

    std::string buildPatternName(const HexPattern& pattern);

    glm::vec4 getPatternBaseColor(HexPattern::Type type);

    std::string typeName() const override;

  private:
    PatternComponent* _patterns = nullptr;

    dzemikk::AssetManager* _assetManager = nullptr;

    dzemikk::GameObject* _canvas = nullptr;
    dzemikk::GameObject* _patternsContainer = nullptr;

    dzemikk::AssetHandle<nlohmann::json> _patternSlotPrefab;
    dzemikk::AssetHandle<nlohmann::json> _hexUIPrefab;

    std::vector<PatternUIEntry> _uiEntries;
    bool _isClickable = false;

};

} // namespace game