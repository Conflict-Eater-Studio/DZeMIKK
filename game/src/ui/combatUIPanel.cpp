#include "ui/combatUIPanel.h"

#include <ecs/gameobject.h>
#include <ecs/scene.h>
#include <ecs/components/ui/gridLayout.h>
#include <ecs/serialize/prefabSerializer.h>
#include <ecs/components/ui/uiActionRegistry.h>
#include <iostream>
#include <enemySystem/enemyPatternComponent.h>

void game::CombatUIPanel::start() {
    _patternSlotPrefab = _assetManager->get<nlohmann::json>("prefabs/pattern_ui.prefab");

    _hexUIPrefab = _assetManager->get<nlohmann::json>("prefabs/hex_ui.prefab");

    _patternsContainer = findPatternsContainer();

    refresh();
}

void game::CombatUIPanel::update(double deltaTime) {}

void game::CombatUIPanel::refresh() {
    clear();

    if (!_patterns)
        return;

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
    if (!_patterns)
        return;

    if (!_patternsContainer)
        return;

    const auto& patterns = _patterns->getPatterns();

    for (size_t i = 0; i < patterns.size(); ++i) {
        createPatternSlot(patterns[i], i);
    }

    auto* grid = _patternsContainer->getComponent<dzemikk::GridLayout>();

    if (grid)
        grid->rebuild();
}

void game::CombatUIPanel::createPatternSlot(const PatternComponent::PatternEntry& entry,
                                            size_t index) {

    auto* patternGO = dzemikk::PrefabSerializer::instantiate(
        *getOwner()->getScene(), *_patternSlotPrefab.get(), _assetManager);

    patternGO->setParent(_patternsContainer);
    patternGO->enabled(false);

    PatternUIEntry uiEntry;
    uiEntry.patternIndex = index;
    uiEntry.root = patternGO;

    auto* button = patternGO->getComponent<dzemikk::UIButton>();
    uiEntry.button = button;

    std::string actionId = "pattern_" + std::to_string(index);

    uint32_t usageCount = 0;

    if (_patterns) {
        if (auto* enemyPatterns = dynamic_cast<game::EnemyPatternComponent*>(_patterns)) {

            const auto& usage = enemyPatterns->getPatternUsage();

            auto it = usage.find(&entry.pattern);
            if (it != usage.end()) {
                usageCount = it->second;
            }
        }
    }

    glm::vec4 baseColor = getPatternBaseColor(entry.pattern.getType());
    glm::vec4 color = applyUsageTint(baseColor, usageCount);

    if (_isClickable) {
        dzemikk::UIActionRegistry::get().registerAction(
            [this, index, actionId](const dzemikk::UIEvent&) {
                if (!_patterns)
                    return;

                _patterns->usePattern(index);
                refreshCounts();
            },
            actionId);
    }

    if (button) {
        if (_isClickable) {
            button->addEventListener(dzemikk::UIEventType::Click, actionId);
            button->setStyle({color, color * 0.75f, color * 0.5f});
        } else {
            button->setStyle({color, color, color});
        }

        button->applyVisualState();
    }

    std::string patternName = buildPatternName(entry.pattern);

    for (auto* child : patternGO->getChildren()) {

        if (child->getName() == "Button_Text") {

            auto* text = child->getComponent<dzemikk::UITextRenderer>();
            if (text) {
                text->text = patternName;
            }
        }

        if (child->getName() == "Count") {

            for (auto* countChild : child->getChildren()) {

                if (countChild->getName() == "Text_Count") {

                    auto* text = countChild->getComponent<dzemikk::UITextRenderer>();
                    if (text) {
                        text->text = std::to_string(usageCount);
                        uiEntry.countText = text;
                    }
                }
            }
        }

        if (child->getName() == "HexHolder") {
            createPatternPreview(child, entry.pattern);
        }
    }

    _uiEntries.push_back(uiEntry);
}

void game::CombatUIPanel::createPatternPreview(dzemikk::GameObject* parent,
                                               const HexPattern& pattern) {

    constexpr float HEX_SIZE = 12.5f;

    std::vector<glm::vec2> positions;

    for (const auto& hex : pattern.getHexes()) {

        int q = hex.q();
        int r = hex.r();

        float x = HEX_SIZE * sqrt(3.f) * (q + r * 0.5f);

        float y = HEX_SIZE * 1.5f * r;

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

    glm::vec2 center = (minPos + maxPos) * 0.5f;

    for (size_t i = 0; i < positions.size(); ++i) {

        auto* hexGO = dzemikk::PrefabSerializer::instantiate(*getOwner()->getScene(),
                                                             *_hexUIPrefab.get(), _assetManager);

        hexGO->setParent(parent);
        hexGO->enabled(false);

        auto* transform = hexGO->rectTransform();

        transform->setPosition(glm::vec3(positions[i] - center, 0.0f));
    }
}

void game::CombatUIPanel::refreshCounts() {
    for (auto& entry : _uiEntries) {

        if (!entry.countText)
            continue;

        const auto& patternEntry = _patterns->getPatterns()[entry.patternIndex];

        int32_t count = getPatternCount(patternEntry);

        entry.countText->text = std::to_string(count);
    }
}

dzemikk::GameObject* game::CombatUIPanel::findPatternsContainer() {

    if (!_canvas)
        return nullptr;

    for (auto* child : _canvas->getChildren()) {

        for (auto* grandChild : child->getChildren()) {

            if (grandChild->getName() == "Patterns") {
                return grandChild;
            }
        }
    }

    return nullptr;
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
        return {1.f, 0.f, 0.f, 1.f};
    case HexPattern::Type::DEF:
        return {0.f, 0.f, 1.f, 1.f};
    case HexPattern::Type::HEAL:
        return {0.f, 1.f, 0.f, 1.f};
    default:
        return {0.3f, 0.3f, 0.3f, 1.f};
    }
}

glm::vec4 game::CombatUIPanel::applyUsageTint(glm::vec4 base, uint32_t count) {
    float intensity = std::min(1.0f, count * 0.2f);

    base.r += intensity * 0.3f;
    base.g += intensity * 0.3f;
    base.b += intensity * 0.3f;

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
    if (!_patterns)
        return 0;

    if (_mode == Mode::AvailablePatterns) {
        return entry.count; 
    }

    const auto* enemyPatterns = dynamic_cast<const EnemyPatternComponent*>(_patterns);

    if (!enemyPatterns)
        return 0;

    const auto& usage = enemyPatterns->getPatternUsage();

    auto it = usage.find(&entry.pattern);

    if (it == usage.end())
        return 0;

    return it->second;
}

void game::CombatUIPanel::refreshVisuals() {
    if (!_patterns)
        return;

    const bool isEnemy = (dynamic_cast<EnemyPatternComponent*>(_patterns) != nullptr);

    for (auto& ui : _uiEntries) {

        if (ui.patternIndex >= _patterns->getPatterns().size())
            continue;

        const auto& entry = _patterns->getPatterns()[ui.patternIndex];

        uint32_t count = 0;

        if (isEnemy) {
            auto* enemy = static_cast<EnemyPatternComponent*>(_patterns);

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
                ui.button->setStyle({color, color * 0.75f, color * 0.5f});
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