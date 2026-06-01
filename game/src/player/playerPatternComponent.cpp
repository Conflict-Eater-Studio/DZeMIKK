#include "player/playerPatternComponent.h"
#include <iostream>
#include <core/engine.h>
#include <game.h>
#include <collisions/collisions.h>
#include <core/window.h>
#include <renderer/renderer.h>
#include <ecs/serialize/prefabSerializer.h>
#include <ecs/scene.h>
#include <assetManager/assetmanager.h>
#include <ecs/components/collider.h>

#include "scripts/world/worldHex.h"
#include <ecs/components/meshRenderer.h>

void game::PlayerPatternComponent::start() {
    // atk1
    addPattern(HexPattern({{-1, 1}, {0, 0}, {1, -1}}, HexPattern::Type::ATK), 1);

    // atk2
    addPattern(HexPattern({{-1, 0}, {0, 0}, {1, -1}}, HexPattern::Type::ATK), -1);

    // atk3
    addPattern(HexPattern({{0, 0}, {1, -1}, {-1, 0}, {0, 1}}, HexPattern::Type::ATK), -1);

    // def1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::DEF), -1);

    // def2
    addPattern(HexPattern({{-1, 1}, {0, 0}, {1, -1}}, HexPattern::Type::DEF), -1);

    // hp1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::HEAL), -1);

    _cancelPatternListenerID = _engine->getInput()->OnMouseButtonPressed.addListener(
        [this](dzemikk::MouseButtonPressedEvent& e) {
            if (e.GetMouseButton() == GLFW_MOUSE_BUTTON_MIDDLE) {
                cancelPattern();
            }
        });

    _rotatePatternListenerID = _engine->getInput()->OnMouseScrolled.addListener([this](dzemikk::MouseScrolledEvent& e) {
        if (_activePatternIndex < 0)
            return;

        auto& pattern = _patterns[_activePatternIndex].pattern;

        if (e.GetYOffset() > 0)
            pattern.rotate(HexPattern::Rotation::Clockwise);
        else
            pattern.rotate(HexPattern::Rotation::CounterClockwise);

        rebuildPreview();
    });

    _confirmPatternListenerID = _engine->getInput()->OnMouseButtonPressed.addListener(
        [this](dzemikk::MouseButtonPressedEvent& e) {
            if (e.GetMouseButton() == GLFW_MOUSE_BUTTON_LEFT) {
                confirmPattern();
            }
        });
}

void game::PlayerPatternComponent::update(double deltaTime) {

    if (_activePatternIndex < 0)
        return;

    int windowWidth = 0;
    int windowHeight = 0;

    glfwGetWindowSize(
        _engine->getWindow()->nativeHandle(),
        &windowWidth,
        &windowHeight);

    dzemikk::Collider* collider =
        _engine->getCollisions()->raycast(
            _engine->getRenderer()->getCameraSystem().getActiveSceneCamera(),
            _engine->getInput()->GetMousePosition(),
            windowWidth,
            windowHeight);

    if (!collider)
        return;

    auto* worldHex =
        collider->getOwner()->getComponent<game::WorldHex>();

    if (!worldHex || !worldHex->getHexCell())
        return;

    auto cell = worldHex->getHexCell();
    _currentPreviewOrigin = cell->getCoord();

    bool validPattern = true;
    _currentPreviewValid = validPattern;

    const auto& pattern =
        _patterns[_activePatternIndex].pattern;

    for (const auto& offset : pattern.getHexes()) {

        HexCoord targetCoord(cell->getCoord().q() + offset.r(), cell->getCoord().r() + offset.q());

        auto targetCell = _grid->getCell(targetCoord);

        if (!targetCell) {
            std::cout << "Missing cell: " << targetCoord.q() << ", " << targetCoord.r()
                      << std::endl;
        }

        std::cout << "Checking: " << targetCoord.q() << ", " << targetCoord.r();

        if (targetCell) {
            std::cout << " type=" << static_cast<int>(targetCell->getType());
        } else {
            std::cout << " NULL";
        }

        std::cout << std::endl;

        if (!targetCell || targetCell->getType() != HexCell::Type::PlayerBattleHex) {
            validPattern = false;
            break;
        }
    }

    glm::vec4 color;

    switch (pattern.getType()) {

    case HexPattern::Type::ATK:
        color = validPattern
                    ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
                    : glm::vec4(1.0f, 0.7f, 0.7f, 1.0f);
        break;

    case HexPattern::Type::DEF:
        color = validPattern
                    ? glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
                    : glm::vec4(0.7f, 0.7f, 1.0f, 1.0f);
        break;

    case HexPattern::Type::HEAL:
        color = validPattern
                    ? glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
                    : glm::vec4(0.7f, 1.0f, 0.7f, 1.0f);
        break;

    default:
        color = glm::vec4(1.0f);
        break;
    }

    auto transform = collider->getOwner()->transform();

    _previewObject->transform()->setPosition(
        {
            transform->getPosition().x,
            transform->getPosition().y + 0.5f,
            transform->getPosition().z
        });

    for (auto* hex : _previewHexes) {

        auto* renderer =
            hex->getComponent<dzemikk::MeshRenderer>();

        if (renderer) {
            renderer->setColor(color);
        }
    }
}

void game::PlayerPatternComponent::onDestroy() {
    _engine->getInput()->OnMouseScrolled.removeListener(_cancelPatternListenerID);
    _engine->getInput()->OnMouseScrolled.removeListener(_rotatePatternListenerID);
}

void game::PlayerPatternComponent::addPattern(const HexPattern& pattern, int count) {
    _patterns.push_back({pattern, count});
}

void game::PlayerPatternComponent::insertPattern(size_t index, const HexPattern& pattern,
                                                 int count) {
    index = std::min(index, _patterns.size());

    _patterns.insert(_patterns.begin() + index, {pattern, count});
}

bool game::PlayerPatternComponent::removePattern(size_t index) {
    if (index >= _patterns.size())
        return false;

    _patterns.erase(_patterns.begin() + index);

    return true;
}

bool game::PlayerPatternComponent::removePattern(const HexPattern& pattern) {
    auto it = std::find_if(_patterns.begin(), _patterns.end(),
                           [&](const PatternEntry& entry) { return entry.pattern == pattern; });

    if (it == _patterns.end())
        return false;

    _patterns.erase(it);

    return true;
}

void game::PlayerPatternComponent::clearPatterns() {
    _patterns.clear();
}

bool game::PlayerPatternComponent::addCount(size_t index, int amount) {
    if (index >= _patterns.size())
        return false;

    _patterns[index].count += amount;

    return true;
}

bool game::PlayerPatternComponent::removeCount(size_t index, int amount) {
    if (index >= _patterns.size())
        return false;

    if (_patterns[index].count < amount)
        return false;

    _patterns[index].count -= amount;

    return true;
}

bool game::PlayerPatternComponent::setCount(size_t index, int count) {
    if (index >= _patterns.size())
        return false;

    _patterns[index].count = count;

    return true;
}

bool game::PlayerPatternComponent::canUsePattern(size_t index) const {
    if (index >= _patterns.size())
        return false;

    return _patterns[index].count != 0;
}

bool game::PlayerPatternComponent::usePattern(size_t index) {
    if (!canUsePattern(index))
        return false;

    auto& entry = _patterns[index];

    if (entry.count > 0) {
        entry.count--;
    }

    _activePatternIndex = static_cast<int>(index);

    auto parent = getOwner()->getScene()->findGameObjectByName("World");
    auto previewPrefab =
        _engine->getAssetManager()->get<nlohmann::json>("prefabs/battle_hex.prefab");

    _previewObject = getOwner()->getScene()->createGameObject();
    _previewObject->setName("PatternPreview");

    const auto& pattern = entry.pattern;
    for (const auto& hex : pattern.getHexes()) {
        auto previewPrefab =
            _engine->getAssetManager()->get<nlohmann::json>("prefabs/battle_hex.prefab");

        auto* hexObject = dzemikk::PrefabSerializer::instantiate(
            *getOwner()->getScene(), *previewPrefab.get(), _engine->getAssetManager());

        hexObject->setParent(_previewObject);

        hexObject->transform()->setPosition(axialToWorld(hex, 1.0f));

        _previewHexes.push_back(hexObject);
    }

    return true;
}

size_t game::PlayerPatternComponent::getPatternCount() const {
    return _patterns.size();
}

game::PlayerPatternComponent::PatternEntry* game::PlayerPatternComponent::getPattern(size_t index) {
    if (index >= _patterns.size())
        return nullptr;

    return &_patterns[index];
}

const game::PlayerPatternComponent::PatternEntry* game::PlayerPatternComponent::getPattern(size_t index) const {
    if (index >= _patterns.size())
        return nullptr;

    return &_patterns[index];
}

const std::vector<game::PlayerPatternComponent::PatternEntry>&
game::PlayerPatternComponent::getPatterns() const {
    return _patterns;
}

int game::PlayerPatternComponent::findPattern(const HexPattern& pattern) const {
    for (size_t i = 0; i < _patterns.size(); ++i) {
        if (_patterns[i].pattern == pattern)
            return static_cast<int>(i);
    }

    return -1;
}

void game::PlayerPatternComponent::setEngine(dzemikk::Engine* engine) {
    _engine = engine;
}

std::string game::PlayerPatternComponent::typeName() const {
    return "PlayerPatternComponent";
}

void game::PlayerPatternComponent::cancelPattern() {
    _activePatternIndex = -1;
    _previewObject->getScene()->destroyGameObject(_previewObject);
}

bool game::PlayerPatternComponent::hasActivePattern() const {
    return _activePatternIndex >= 0;
}

void game::PlayerPatternComponent::clearActivePattern() {
    _activePatternIndex = -1;
}

const game::PlayerPatternComponent::PatternEntry* game::PlayerPatternComponent::getActivePattern() const {

    if (_activePatternIndex < 0 || _activePatternIndex >= static_cast<int>(_patterns.size())) {
        return nullptr;
    }

    return &_patterns[_activePatternIndex];
}

void game::PlayerPatternComponent::setGrid(game::HexGrid* grid) {
    _grid = grid;
}

glm::vec3 game::PlayerPatternComponent::axialToWorld(const HexCoord& coord, float hexSize) {
    float x = hexSize * std::sqrt(3.0f) * (coord.q() + coord.r() * 0.5f);

    float z = hexSize * 1.5f * coord.r();

    return {x, 0.0f, z};
}

void game::PlayerPatternComponent::rebuildPreview() {
    if (!_previewObject || _activePatternIndex < 0)
        return;

    for (auto* hex : _previewHexes) {
        if (hex && hex->getScene())
            hex->getScene()->destroyGameObject(hex);
    }

    _previewHexes.clear();

    const auto& pattern = _patterns[_activePatternIndex].pattern;

    const auto& hexes = pattern.getHexes();

    for (const auto& hex : hexes) {
        auto previewPrefab =
            _engine->getAssetManager()->get<nlohmann::json>("prefabs/battle_hex.prefab");

        auto* hexObject = dzemikk::PrefabSerializer::instantiate(
            *getOwner()->getScene(), *previewPrefab.get(), _engine->getAssetManager());

        hexObject->setParent(_previewObject);

        hexObject->transform()->setPosition(axialToWorld(hex, 1.0f));

        _previewHexes.push_back(hexObject);
    }
}

bool game::PlayerPatternComponent::confirmPattern() {
    if (_activePatternIndex < 0)
        return false;

    if (!_currentPreviewValid)
        return false;

    PlacedPattern placed(_patterns[_activePatternIndex].pattern, _currentPreviewOrigin);

    _placedPatterns.push_back(placed);
    
    for (dzemikk::GameObject* object : _previewHexes) {
        if (!object)
            continue;

        _confirmedHexes.push_back(object);
    }

    _activePatternIndex = -1;

    return true;
}

void game::PlayerPatternComponent::clearPlacedPatterns() {
    _placedPatterns.clear();
}