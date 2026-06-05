#include "ui/combatUIPanel.h"
#include "enemySystem/enemyPatternComponent.h"

#include <ecs/gameobject.h>
#include <ecs/scene.h>
#include <ecs/components/ui/gridLayout.h>
#include <ecs/serialize/prefabSerializer.h>
#include <ecs/components/ui/uiActionRegistry.h>

#include <iostream>

void game::CombatUIPanel::start() {
    _patternSlotPrefab = _assetManager->get<nlohmann::json>("prefabs/pattern_ui.prefab");

    _hexUIPrefab = _assetManager->get<nlohmann::json>("prefabs/hex_ui.prefab");

    _patternsContainer = _canvas->findDescendantByName("Patterns");

    refresh();
}

void game::CombatUIPanel::update(double deltaTime) {}

void game::CombatUIPanel::refresh() {
    clear();

    if (!_patterns) {
        return;
    }

    buildUI();
    refreshCounts();
}

void game::CombatUIPanel::clear() {
    for (auto& entry : _uiEntries) {
        if (entry.root) {
            entry.root->getScene()->destroyGameObject(entry.root);
        }
    }

    _uiEntries.clear();
}

void game::CombatUIPanel::buildUI() {
    if (!_patterns || !_patternsContainer) {
        return;
    }

    const auto& patterns = _patterns->getPatterns();

    for (size_t i = 0; i < patterns.size(); ++i) {
        createPatternSlot(patterns[i], i);
    }

    auto* grid = _patternsContainer->getComponent<dzemikk::GridLayout>();

    if (grid) {
        grid->rebuild();
    }
}

void game::CombatUIPanel::createPatternSlot(const PatternComponent::PatternEntry& entry,
                                            size_t index) {

    auto* patternGO = createPatternSlotObject();

    PatternUIEntry uiEntry;
    uiEntry.patternIndex = index;
    uiEntry.root = patternGO;
    uiEntry.button = patternGO->getComponent<dzemikk::UIButton>();

    const uint32_t usageCount = getUsageCount(entry);

    const auto color = applyUsageTint(getPatternBaseColor(entry.pattern.getType()), usageCount);

    const std::string actionId = "pattern_" + std::to_string(index);

    setupButton(uiEntry.button, index, actionId, color);

    setupPatternSlotContent(patternGO, uiEntry, entry.pattern, usageCount);

    _uiEntries.push_back(uiEntry);
}

void game::CombatUIPanel::createPatternPreview(dzemikk::GameObject* parent,
                                               const HexPattern& pattern) {
    constexpr float HEX_SIZE = 15.0F;

    std::vector<glm::vec2> positions;

    for (const auto& hex : pattern.getHexes()) {
        float x =
            HEX_SIZE * std::numbers::sqrt3_v<float> * ((float)hex.q() + (float)hex.r() * 0.5F);
        float y = HEX_SIZE * 1.5F * (float)hex.r();
        positions.emplace_back(-y, x);
    }

    glm::vec2 minPos(FLT_MAX);
    glm::vec2 maxPos(-FLT_MAX);

    for (auto& p : positions) {

        minPos.x = std::min(minPos.x, p.x);
        minPos.y = std::min(minPos.y, p.y);

        maxPos.x = std::max(maxPos.x, p.x);
        maxPos.y = std::max(maxPos.y, p.y);
    }

    glm::vec2 center = (minPos + maxPos) * 0.5F;

    for (size_t i = 0; i < positions.size(); ++i) {

        auto* hexGO = dzemikk::PrefabSerializer::instantiate(*getOwner()->getScene(),
                                                             *_hexUIPrefab.get(), _assetManager);
        hexGO->setParent(parent);
        hexGO->enabled(false);

        auto* transform = hexGO->rectTransform();
        transform->setPosition(glm::vec3(positions[i] - center, 0.0F));
    }
}

void game::CombatUIPanel::refreshCounts() {
    for (auto& entry : _uiEntries) {
        if (!entry.countText) {
            continue;
        }

        const auto& patternEntry = _patterns->getPatterns()[entry.patternIndex];
        int32_t count = getPatternCount(patternEntry);
        entry.countText->text = std::to_string(count);
    }
}

std::string game::CombatUIPanel::buildPatternName(const HexPattern& pattern) {

    switch (pattern.getType()) {

    case HexPattern::Type::ATK:
        return "Attack";

    case HexPattern::Type::DEF:
        return "Defense";

    case HexPattern::Type::HEAL:
        return "Heal";

    default:
        return "Pattern";
    }
}

glm::vec4 game::CombatUIPanel::getPatternBaseColor(HexPattern::Type type) {
    switch (type) {
    case HexPattern::Type::ATK:
        return {1.F, 0.F, 0.F, 1.F};
    case HexPattern::Type::DEF:
        return {0.F, 0.F, 1.F, 1.F};
    case HexPattern::Type::HEAL:
        return {0.F, 1.F, 0.F, 1.F};
    default:
        return {0.3F, 0.3F, 0.3F, 1.F};
    }
}

glm::vec4 game::CombatUIPanel::applyUsageTint(glm::vec4 base, uint32_t count) {
    float intensity = std::min(1.0F, (float)count * 0.2F);

    base.r += intensity * 0.3F;
    base.g += intensity * 0.3F;
    base.b += intensity * 0.3F;

    return base;
}

std::string game::CombatUIPanel::typeName() const {
    return "CombatUIPanel";
}

void game::CombatUIPanel::setPatternsComponent(PatternComponent* patterns) {
    _patterns = patterns;
}

void game::CombatUIPanel::setAssetManager(dzemikk::AssetManager* assetManager) {
    _assetManager = assetManager;
}

void game::CombatUIPanel::setCanvas(dzemikk::GameObject* canvas) {
    _canvas = canvas;
}

void game::CombatUIPanel::setMode(Mode mode) {
    _mode = mode;
    refresh();
}

int32_t game::CombatUIPanel::getPatternCount(const PatternComponent::PatternEntry& entry) const {
    if (!_patterns) {
        return 0;
    }

    if (_mode == Mode::AvailablePatterns) {
        return entry.count; 
    }

    const auto* enemyPatterns = dynamic_cast<const EnemyPatternComponent*>(_patterns);

    if (!enemyPatterns) {
        return 0;
    }

    const auto& usage = enemyPatterns->getPatternUsage();

    auto it = usage.find(&entry.pattern);

    if (it == usage.end()) {
        return 0;
    }

    return (int32_t)it->second;
}

void game::CombatUIPanel::refreshVisuals() {
    if (!_patterns) {
        return;
    }

    const bool isEnemy = (dynamic_cast<EnemyPatternComponent*>(_patterns) != nullptr);

    for (auto& ui : _uiEntries) {
        if (ui.patternIndex >= _patterns->getPatterns().size()) {
            continue;
        }

        const auto& entry = _patterns->getPatterns()[ui.patternIndex];

        uint32_t count = 0;

        if (isEnemy) {
            auto* enemy = dynamic_cast<EnemyPatternComponent*>(_patterns);

            const auto& usage = enemy->getPatternUsage();

            auto it = usage.find(&entry.pattern);
            if (it != usage.end()) {
                count = it->second;
            }
        } else {
            count = entry.count;
        }

        glm::vec4 baseColor = getPatternBaseColor(entry.pattern.getType());
        glm::vec4 color = applyUsageTint(baseColor, count);

        if (ui.button) {
            if (_isClickable) {
                ui.button->setStyle({color, color * 0.75F, color * 0.5F});
            } else {
                ui.button->setStyle({color, color, color});
            }

            ui.button->applyVisualState();
        }

        if (ui.countText) {
            ui.countText->text = std::to_string(count);
        }
    }
}

dzemikk::GameObject* game::CombatUIPanel::createPatternSlotObject() {
    auto* patternGO = dzemikk::PrefabSerializer::instantiate(
        *getOwner()->getScene(), *_patternSlotPrefab.get(), _assetManager);

    patternGO->setParent(_patternsContainer);
    patternGO->enabled(false);

    return patternGO;
}

uint32_t game::CombatUIPanel::getUsageCount(const PatternComponent::PatternEntry& entry) const {
    if (!_patterns) {
        return 0;
    }

    auto* enemyPatterns = dynamic_cast<game::EnemyPatternComponent*>(_patterns);

    if (!enemyPatterns) {
        return 0;
    }

    const auto& usage = enemyPatterns->getPatternUsage();

    auto it = usage.find(&entry.pattern);

    return it != usage.end() ? it->second : 0;
}

void game::CombatUIPanel::setupButton(dzemikk::UIButton* button, size_t index,
                                      const std::string& actionId, const glm::vec4& color) {
    if (!button) {
        return;
    }

    if (_isClickable) {

        dzemikk::UIActionRegistry::get().registerAction(
            [this, index, actionId](const dzemikk::UIEvent&) {
                if (!_patterns) {
                    return;
                }

                _patterns->usePattern(index);
                refreshCounts();
            },
            actionId);

        button->addEventListener(dzemikk::UIEventType::Click, actionId);

        button->setStyle({color, color * 0.75F, color * 0.5F});
    } else {
        button->setStyle({color, color, color});
    }

    button->applyVisualState();
}

void game::CombatUIPanel::setupPatternSlotContent(dzemikk::GameObject* patternGO,
                                                  PatternUIEntry& uiEntry,
                                                  const HexPattern& pattern, uint32_t usageCount) {
    const std::string patternName = buildPatternName(pattern);

    for (auto* child : patternGO->getChildren()) {

        if (child->getName() == "Button_Text") {
            setupPatternName(child, patternName);
        } else if (child->getName() == "Count") {
            setupCountText(child, usageCount, uiEntry);
        } else if (child->getName() == "HexHolder") {
            createPatternPreview(child, pattern);
        }
    }
}

void game::CombatUIPanel::setupPatternName(dzemikk::GameObject* object,
                                           const std::string& patternName) {
    auto* text = object->getComponent<dzemikk::UITextRenderer>();

    if (text) {
        text->text = patternName;
    }
}

void game::CombatUIPanel::setupCountText(dzemikk::GameObject* countRoot, uint32_t usageCount,
                                         PatternUIEntry& uiEntry) {
    for (auto* child : countRoot->getChildren()) {

        if (child->getName() != "Text_Count") {
            continue;
        }

        auto* text = child->getComponent<dzemikk::UITextRenderer>();

        if (!text) {
            continue;
        }

        text->text = std::to_string(usageCount);
        uiEntry.countText = text;
    }
}