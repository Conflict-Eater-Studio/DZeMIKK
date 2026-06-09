#ifndef GAME_COMBAT_UI_PANEL_H
#define GAME_COMBAT_UI_PANEL_H

#include "enemySystem/patternComponent.h"

#include <assetManager/assetmanager.h>
#include <ecs/components/monobehaviour.h>
#include <ecs/components/ui/uiButton.h>
#include <nlohmann/json.hpp>

namespace game {

/**
 * @brief UI panel used to display combat patterns.
 *
 * Depending on the selected mode, the panel displays either
 * patterns currently available to the player or statistics
 * about enemy pattern usage.
 */
class CombatUIPanel : public dzemikk::MonoBehaviour {
  public:
    /**
     * @brief UI elements associated with a single pattern.
     */
    struct PatternUIEntry {

        /** Index of the represented pattern. */
        size_t patternIndex = 0;

        /** Root object of the UI entry. */
        dzemikk::GameObject* root = nullptr;

        /** Button used to select the pattern. */
        dzemikk::UIButton* button = nullptr;

        /** Text displaying remaining count or usage information. */
        dzemikk::UITextRenderer* countText = nullptr;
    };

    /**
     * @brief Display mode of the combat panel.
     */
    enum class Mode : std::uint8_t { AvailablePatterns, EnemyUsage };

    /**
     * @brief Creates a combat UI panel.
     *
     * @param isClickable Whether pattern entries can be selected.
     * @param mode Initial panel mode.
     */
    CombatUIPanel(bool isClickable, Mode mode) : _isClickable(isClickable), _mode(mode) {}

    void start() override;
    void update(double deltaTime) override;

    /**
     * @brief Rebuilds the panel contents.
     */
    void refresh();

    /**
     * @brief Removes all generated UI entries.
     */
    void clear();

    /**
     * @brief Sets the source of pattern data.
     */
    void setPatternsComponent(PatternComponent* patterns);

    /**
     * @brief Sets the asset manager used for prefab loading.
     */
    void setAssetManager(dzemikk::AssetManager* assetManager);

    /**
     * @brief Sets the parent canvas for generated UI.
     */
    void setCanvas(dzemikk::GameObject* canvas);

    /**
     * @brief Updates visual state of all entries.
     */
    void refreshVisuals();
    /**
     * @brief Creates the panel UI hierarchy.
     */
    void buildUI();

    void addPatternSlot(const PatternComponent::PatternEntry& entry);

  private:
    /**
     * @brief Creates a UI slot for a pattern.
     */
    void createPatternSlot(const PatternComponent::PatternEntry& entry, size_t index);

    /**
     * @brief Creates a visual preview of a hex pattern.
     */
    void createPatternPreview(dzemikk::GameObject* parent, const HexPattern& pattern);

    /**
     * @brief Refreshes displayed counts.
     */
    void refreshCounts();

    /**
     * @brief Builds a user-friendly pattern name.
     */
    static std::string buildPatternName(const HexPattern& pattern);

    /**
     * @brief Returns the base color associated with a pattern type.
     */
    static glm::vec4 getPatternBaseColor(HexPattern::Type type);

    [[nodiscard]] std::string typeName() const override;

    /**
     * @brief Changes panel display mode.
     */
    void setMode(Mode mode);

    /**
     * @brief Returns the displayed count for a pattern entry.
     */
    [[nodiscard]] int32_t getPatternCount(const PatternComponent::PatternEntry& entry) const;

    /**
     * @brief Applies usage-based tinting to a color.
     */
    static glm::vec4 applyUsageTint(glm::vec4 base, uint32_t count);

    /**
     * @brief Configures button behavior and visual style.
     *
     * Registers click actions when the panel is interactive and
     * applies the appropriate visual state colors.
     */
    void setupButton(dzemikk::UIButton* button, size_t index, const std::string& actionId,
                     const glm::vec4& color);

    /**
     * @brief Creates a pattern slot instance from the slot prefab.
     *
     * The created object is attached to the patterns container
     * and initially disabled until fully configured.
     *
     * @return Root object of the created slot.
     */
    dzemikk::GameObject* createPatternSlotObject();

    /**
     * @brief Returns enemy usage count for a pattern.
     *
     * Used only when the panel is displaying enemy pattern
     * statistics.
     *
     * @param entry Pattern entry to query.
     * @return Number of times the pattern has been used.
     */
    [[nodiscard]] uint32_t getUsageCount(const PatternComponent::PatternEntry& entry) const;

    /**
     * @brief Populates a pattern slot with texts and preview visuals.
     *
     * Updates the pattern name, usage/count display and creates
     * the hex preview representation.
     */
    void setupPatternSlotContent(dzemikk::GameObject* patternGO, PatternUIEntry& uiEntry,
                                 const HexPattern& pattern, uint32_t usageCount);

    /**
     * @brief Sets the display name of a pattern slot.
     *
     * @param object UI object containing the text renderer.
     * @param patternName Human-readable pattern name.
     */
    static void setupPatternName(dzemikk::GameObject* object, const std::string& patternName);

    /**
     * @brief Updates the usage/count text of a pattern slot.
     *
     * Stores the discovered text renderer inside the UI entry
     * for later updates.
     *
     * @param countRoot Parent object containing the count text.
     * @param usageCount Value to display.
     * @param uiEntry Associated UI entry.
     */
    static void setupCountText(dzemikk::GameObject* countRoot, uint32_t usageCount,
                               PatternUIEntry& uiEntry);

    /** Source of pattern data. */
    PatternComponent* _patterns = nullptr;

    /** Asset manager used for prefab loading. */
    dzemikk::AssetManager* _assetManager = nullptr;

    /** Parent canvas. */
    dzemikk::GameObject* _canvas = nullptr;

    /** Container holding generated entries. */
    dzemikk::GameObject* _patternsContainer = nullptr;

    /** Current panel mode. */
    Mode _mode = Mode::AvailablePatterns;

    /** Pattern slot prefab. */
    dzemikk::AssetHandle<nlohmann::json> _patternSlotPrefab;

    /** Hex preview prefab. */
    dzemikk::AssetHandle<nlohmann::json> _hexUIPrefab;

    /** Generated UI entries. */
    std::vector<PatternUIEntry> _uiEntries;

    /** Whether pattern buttons can be interacted with. */
    bool _isClickable = false;
};

} // namespace game

#endif
