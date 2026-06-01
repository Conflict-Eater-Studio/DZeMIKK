#include "ui/combatUIPanel.h"

#include <ecs/gameobject.h>
#include <ecs/scene.h>
#include <ecs/components/ui/gridLayout.h>
#include <ecs/serialize/prefabSerializer.h>
#include <ecs/components/ui/uiActionRegistry.h>
#include <iostream>

void game::CombatUIPanel::start() {
    _patternSlotPrefab = _assetManager->get<nlohmann::json>("prefabs/pattern_ui.prefab");

    _hexUIPrefab = _assetManager->get<nlohmann::json>("prefabs/hex_ui.prefab");

    _patternsContainer = findPatternsContainer();

    refresh();
}

void game::CombatUIPanel::update(double deltaTime) {}

void game::CombatUIPanel::refresh() {
    clear();
    buildUI();
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

void game::CombatUIPanel::createPatternSlot(const PlayerPatternComponent::PatternEntry& entry,
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

    dzemikk::UIActionRegistry::get().registerAction(
        [this, index, actionId](const dzemikk::UIEvent&) {
            if (!_patterns)
                return;

            _patterns->usePattern(index);

            refreshCounts();
        },
        actionId);

    if (button) {
        button->addEventListener(dzemikk::UIEventType::Click, actionId);

        glm::vec4 baseColor = getPatternBaseColor(entry.pattern.getType());

        button->setStyle({baseColor, baseColor * 0.75f, baseColor * 0.5f});

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
                        text->text = std::to_string(entry.count);

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

    if (!_patterns)
        return;

    for (auto& entry : _uiEntries) {

        auto* pattern = _patterns->getPattern(entry.patternIndex);

        if (!pattern)
            continue;

        if (!entry.countText)
            continue;

        entry.countText->text = std::to_string(pattern->count);
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
        return {0.6f, 0.0f, 0.0f, 1.0f};

    case HexPattern::Type::DEF:
        return {0.0f, 0.0f, 0.6f, 1.0f};

    case HexPattern::Type::HEAL:
        return {0.0f, 0.6f, 0.0f, 1.0f};

    default:
        return {0.3f, 0.3f, 0.3f, 1.0f};
    }
}

std::string game::CombatUIPanel::typeName() const {
    return "CombatUIPanel";
}

void game::CombatUIPanel::setPlayerPatterns(PlayerPatternComponent* patterns) {
    _patterns = patterns;
}

void game::CombatUIPanel::setAssetManager(dzemikk::AssetManager* assetManager) {
    _assetManager = assetManager;
}

void game::CombatUIPanel::setCanvas(dzemikk::GameObject* canvas) {
    _canvas = canvas;
}