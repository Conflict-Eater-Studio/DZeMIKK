#include "player/playerPatternComponent.h"

#include "ecs/gameobject.h"
#include "assetManager/assetmanager.h"
#include <ecs/serialize/prefabSerializer.h>
#include <ecs/components/ui/gridLayout.h>
#include <ecs/components/ui/uiTextRenderer.h>
#include <ecs/components/ui/uiButton.h>
#include <ecs/components/transform.h>
#include <iostream>

void game::PlayerPatternComponent::start() {
    // atk1
    addPattern(HexPattern({{0, 0}, {1, -1}, {2, -2}}, HexPattern::Type::ATK), -1);

    // atk2
    addPattern(HexPattern({{0, 0}, {1, 0}, {2, -1}}, HexPattern::Type::ATK), -1);

    // atk3
    addPattern(HexPattern({{0, 0}, {1, -1}, {-1, 0}, {0, 1}}, HexPattern::Type::ATK), -1);

    // def1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::DEF), -1);

    // def2
    addPattern(HexPattern({{0, 0}, {1, -1}, {2, -2}}, HexPattern::Type::DEF), -1);

    // hp1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::HEAL), -1);

    _patternSlotPrefab = _assetManager->get<nlohmann::json>("prefabs/pattern_ui.prefab");
    _hexUIPrefab = _assetManager->get<nlohmann::json>("prefabs/hex_ui.prefab");
    setupUI();
}

void game::PlayerPatternComponent::update(double deltaTime) {}

void game::PlayerPatternComponent::addPattern(const HexPattern& pattern, int count) {
    _patterns.push_back({pattern, count});
}

bool game::PlayerPatternComponent::canUsePattern(size_t index) const {
    if (index >= _patterns.size())
        return false;

    return _patterns[index].count > 0;
}

std::vector<game::HexCell*>
game::PlayerPatternComponent::getAffectedCells(size_t index, HexPattern::Rotation rotation) const {
    return std::vector<HexCell*>();
}

void game::PlayerPatternComponent::usePattern(size_t index) {
    if (!canUsePattern(index))
        return;

    _patterns[index].count--;
}

std::string game::PlayerPatternComponent::typeName() const {
    return "PlayerPatternComponent";
}

void game::PlayerPatternComponent::setupUI() {
    dzemikk::GameObject* patternsGO = nullptr;

    for (auto* child : _playerPatternsCanvas->getChildren()) {
        for (auto* grandChild : child->getChildren()) {
            if (grandChild->getName() == "Patterns") {
                patternsGO = grandChild;
                break;
            }
        }

        if (patternsGO)
            break;
    }

    if (!patternsGO)
        return;

    int attackIndex = 1;
    int healIndex = 1;
    int supportIndex = 1;

    for (const auto& entry : _patterns) {
        auto* patternGO = dzemikk::PrefabSerializer::instantiate(
            *_playerPatternsCanvas->getScene(), *_patternSlotPrefab.get(), _assetManager);

        patternGO->setParent(patternsGO);
        patternGO->enabled(false);

        std::string patternName;
        auto button = patternGO->getComponent<dzemikk::UIButton>();

        switch (entry.pattern.getType()) {
        case HexPattern::Type::ATK:
            patternName = "Attack " + std::to_string(attackIndex++);
            button->setStyle(
                {glm::vec4(0.6, 0, 0, 1), glm::vec4(0.4, 0, 0, 1), glm::vec4(0.24, 0, 0, 1)});
            break;

        case HexPattern::Type::HEAL:
            patternName = "Heal " + std::to_string(healIndex++);
            button->setStyle(
                {glm::vec4(0, 0.6, 0, 1), glm::vec4(0, 0.4, 0, 1), glm::vec4(0, 0.24, 0, 1)});
            break;

        case HexPattern::Type::DEF:
            patternName = "Defense" + std::to_string(supportIndex++);
            button->setStyle(
                {glm::vec4(0, 0, 0.6, 1), glm::vec4(0, 0, 0.4, 1), glm::vec4(0, 0, 0.24, 1)});
            break;

        default:
            patternName = "Pattern";
            break;
        }

        button->applyVisualState();


        for (auto* child : patternGO->getChildren()) {
            // Button_Text
            if (child->getName() == "Button_Text") {
                auto* text = child->getComponent<dzemikk::UITextRenderer>();

                if (text)
                    text->text = patternName;
            }

            // Count
            if (child->getName() == "Count") {
                for (auto* countChild : child->getChildren()) {
                    if (countChild->getName() == "Text_Count") {
                        auto* text = countChild->getComponent<dzemikk::UITextRenderer>();

                        if (text)
                            text->text = std::to_string(entry.count);
                    }
                }
            }

            // HexHolder
            if (child->getName() == "HexHolder") {
                constexpr float HEX_SIZE = 12.5f;

                std::vector<glm::vec2> positions;
                positions.reserve(entry.pattern.getHexes().size());

                for (const auto& hex : entry.pattern.getHexes()) {
                    const int q = hex.q();
                    const int r = hex.r();

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

                for (size_t i = 0; i < entry.pattern.getHexes().size(); i++) {
                    auto* hexGO = dzemikk::PrefabSerializer::instantiate(
                        *_playerPatternsCanvas->getScene(), *_hexUIPrefab.get(), _assetManager);

                    hexGO->setParent(child);
                    hexGO->enabled(false);

                    auto* transform = hexGO->rectTransform();

                    transform->setPosition(glm::vec3(positions[i] - center, 0.0f));
                }
            }
        }
    }

    auto* grid = patternsGO->getComponent<dzemikk::GridLayout>();

    if (grid)
        grid->rebuild();
}

void game::PlayerPatternComponent::setPlayer(game::PlayerEntity* player) {
    _player = player;
}

game::PlayerEntity* game::PlayerPatternComponent::getPlayer() const {
    return _player;
}

void game::PlayerPatternComponent::setGrid(game::HexGrid* grid) {
    _grid = grid;
}

game::HexGrid* game::PlayerPatternComponent::getGrid() const {
    return _grid;
}

void game::PlayerPatternComponent::setPlayerPatternsCanvas(GameObject* playerPatternsCanvas) {
    _playerPatternsCanvas = playerPatternsCanvas;
}

GameObject* game::PlayerPatternComponent::getPlayerPatternsCanvas() const {
    return _playerPatternsCanvas;
}

void game::PlayerPatternComponent::setAssetManager(dzemikk::AssetManager* assetManager) {
    _assetManager = assetManager;
}