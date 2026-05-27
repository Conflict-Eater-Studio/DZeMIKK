#include "player/playerPatternComponent.h"

#include "ecs/gameobject.h"
#include "assetManager/assetmanager.h"
#include <ecs/serialize/prefabSerializer.h>
#include <ecs/components/ui/gridLayout.h>

void game::PlayerPatternComponent::start() {

    // atk1
    addPattern(HexPattern({{0, 0}, {1, -1}, {2, -2}}, HexPattern::Type::ATK), 1);

    // atk2
    addPattern(HexPattern({{0, 0}, {1, 0}, {1, -1}}, HexPattern::Type::ATK), 1);

    // atk3
    addPattern(HexPattern({{0, 0}, {1, 0}, {-1, 0}, {0, 1}}, HexPattern::Type::ATK), 1);

    // def1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::DEF), 1);

    // def2
    addPattern(HexPattern({{0, 0}, {1, -1}, {2, -2}}, HexPattern::Type::DEF), 1);

    // hp1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::HEAL), 1);

    _patternSlotPrefab = _assetManager->get<nlohmann::json>("prefabs/pattern_ui.prefab");
    _hexUIPrefab = _assetManager->get<nlohmann::json>("prefabs/hex_ui.prefab");
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

    for (int i = 0; i < _patterns.size(); i++) {
        auto* patternGO = dzemikk::PrefabSerializer::instantiate(
            *_playerPatternsCanvas->getScene(), *_patternSlotPrefab.get(), _assetManager);
        patternGO->setParent(patternsGO);
    }

    auto* grid = patternsGO->getComponent<dzemikk::GridLayout>();
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