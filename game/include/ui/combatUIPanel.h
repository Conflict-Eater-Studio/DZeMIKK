#ifndef GAME_COMBAT_UI_PANEL_H
#define GAME_COMBAT_UI_PANEL_H

#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/gameobject.h"
#include "enemySystem/patternComponent.h"

#include <assetManager/assetmanager.h>
#include <ecs/components/monobehaviour.h>
#include <ecs/components/ui/uiButton.h>
#include <events/mouse_event.h>
#include <nlohmann/json.hpp>

namespace dzemikk {
class Engine;
}

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
     * @brief UI elements associated with a pooled pattern slot.
     *
     * Used to store references to UI components of a pattern slot
     * for later reuse when refreshing the panel.
     */
    struct PatternPoolObject {
        dzemikk::UIButton* button = nullptr;
        dzemikk::GameObject* root = nullptr;
        dzemikk::UITextRenderer* countText = nullptr;
        dzemikk::UITextRenderer* nameText = nullptr;
        dzemikk::GameObject* previewContainer = nullptr;
        dzemikk::ImageRenderer* borderRenderer = nullptr;
        size_t patternIndex = 0;
        bool used = false;
        std::vector<std::pair<dzemikk::GameObject* /*hex preview*/, bool /*used*/>> hexPool;
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
    void onDestroy() override;

    /**
     * @brief Rebuilds the panel contents.
     */
    void refresh(bool enableChildren = false);
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
    void buildUI();
    /**
     * @brief Refreshes displayed counts.
     */
    void refreshCounts();
    void setHideEmptyPatterns(bool value);
    void setEngine(dzemikk::Engine* engine);

  private:
    /**
     * @brief Creates a UI slot for a pattern.
     */
    void createPatternSlot(const PatternComponent::PatternEntry& entry, size_t index);
    /**
     * @brief Creates a visual preview of a hex pattern.
     */
    void createPatternPreview(PatternPoolObject& obj, const HexPattern& pattern);
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
     * @brief Registers button click/hover/unhover actions once per pooled slot.
     *
     * Actions are bound to a fixed pool slot and read patternIndex at click time,
     * so they stay correct when the slot is reused for different patterns.
     */
    void setupButtonActions(PatternPoolObject& obj, size_t poolIndex);

    /**
     * @brief Configures button visual style.
     *
     * Applies the appropriate visual state colors. Does not register actions.
     */
    void setupButton(PatternPoolObject& obj, const glm::vec4& color);
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
    void setupPatternSlotContent(PatternPoolObject& obj, const HexPattern& pattern,
                                 uint32_t usageCount);

    [[nodiscard]] std::vector<size_t> getAvailablePatternIndices() const;

    void onMouseScrolled(dzemikk::MouseScrolledEvent& e);
    [[nodiscard]] float calculateScrollHandleY() const;
    void updateScrollHandle();
    [[nodiscard]] bool isMouseOverPanel() const;

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
    /** Whether pattern buttons can be interacted with. */
    bool _isClickable = false;
    bool _hideEmptyPatterns = false;

    // --- Pooling ---
    std::vector<PatternPoolObject> _patternPool;

    static constexpr size_t kPatternsPerRow = 2;
    static constexpr size_t kMaxVisiblePatterns = 8;

    dzemikk::Engine* _engine = nullptr;

    size_t _firstVisiblePatternIndex = 0;
    uint64_t _scrollListenerId = 0;
    dzemikk::GameObject* _scrollHandle = nullptr;
    float _scrollHandleMinY = -240.0F;
    float _scrollHandleMaxY = 240.0F;
};
} // namespace game
#endif
