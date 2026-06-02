#include "player/playerPatternComponent.h"
#include "player/playerPatternStatsComponent.h"
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

    _playerPatternStats = getOwner()->getComponent<PlayerPatternStatsComponent>();

    _cancelPatternListenerID = _engine->getInput()->OnMouseButtonPressed.addListener(
        [this](dzemikk::MouseButtonPressedEvent& e) {
            if (e.GetMouseButton() == GLFW_MOUSE_BUTTON_RIGHT) {
                if (_activePatternIndex >= 0) {
                    cancelPattern();
                }

                tryRemovePlacedPatternUnderCursor();
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

        if (!targetCell || targetCell->getType() != HexCell::Type::PlayerBattleHex ||
            isCellOccupiedByPattern(targetCoord)) {
            validPattern = false;
            break;
        }
    }

    _currentPreviewValid = validPattern;

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
            transform->getPosition().y + 2.f,
            transform->getPosition().z
        });

    for (auto* hex : _previewHexes) {

        auto* renderer =
            hex->getComponent<dzemikk::MeshRenderer>();

        if (renderer) {
            renderer->setColor(color);
            renderer->setCullingRadius(50.0F);
        }
    }
}

void game::PlayerPatternComponent::onDestroy() {
    _engine->getInput()->OnMouseButtonPressed.removeListener(_cancelPatternListenerID);
    _engine->getInput()->OnMouseScrolled.removeListener(_rotatePatternListenerID);
    _engine->getInput()->OnMouseButtonPressed.removeListener(_confirmPatternListenerID);
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

bool game::PlayerPatternComponent::usePattern(size_t index) {
    if (!canUsePattern(index))
        return false;

    auto& entry = _patterns[index];

    if (entry.count > 0) {
        entry.count--;
    }

    _activePatternIndex = static_cast<int>(index);
    destroyPreview();

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

void game::PlayerPatternComponent::setEngine(dzemikk::Engine* engine) {
    _engine = engine;
}

std::string game::PlayerPatternComponent::typeName() const {
    return "PlayerPatternComponent";
}

bool game::PlayerPatternComponent::hasActivePattern() const {
    return _activePatternIndex >= 0;
}

void game::PlayerPatternComponent::clearActivePattern() {
    _activePatternIndex = -1;
}

const game::PlayerPatternComponent::PatternEntry*
game::PlayerPatternComponent::getActivePattern() const {

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

void game::PlayerPatternComponent::cancelPattern() {
    _activePatternIndex = -1;

    if (_previewObject)
        _previewObject->getScene()->destroyGameObject(_previewObject);

    _previewObject = nullptr;
    _previewHexes.clear();
}

bool game::PlayerPatternComponent::confirmPattern() {
    if (_activePatternIndex < 0)
        return false;

    if (!_currentPreviewValid)
        return false;

    PlacedPattern placed(_patterns[_activePatternIndex].pattern, _currentPreviewOrigin);

    for (auto* object : _previewHexes) {
        if (!object)
            continue;

        placed.objects.push_back(object);
    }

    _placedPatterns.push_back(placed);

    if (_playerPatternStats) {
        _playerPatternStats->registerPlacement(placed.pattern);
    }

    _previewHexes.clear();
    _previewObject = nullptr;

    restartPreview();

    return true;
}

void game::PlayerPatternComponent::clearPlacedPatterns() {

    for (auto& placed : _placedPatterns) {

        for (auto* obj : placed.objects) {
            if (obj && obj->getScene()) {
                obj->getScene()->destroyGameObject(obj);
            }
        }

        placed.objects.clear();
    }

    _placedPatterns.clear();
}

bool game::PlayerPatternComponent::isCellOccupiedByPattern(const HexCoord& coord) const {
    for (const auto& placed : _placedPatterns) {
        for (const auto& offset : placed.pattern.getHexes()) {
            HexCoord occupied(placed.origin.q() + offset.r(), placed.origin.r() + offset.q());

            if (occupied == coord)
                return true;
        }
    }

    return false;
}

void game::PlayerPatternComponent::clearPreview() {
    _activePatternIndex = -1;

    if (_previewObject) {
        _previewObject->getScene()->destroyGameObject(_previewObject);
        _previewObject = nullptr;
    }

    _previewHexes.clear();
}

void game::PlayerPatternComponent::restartPreview() {
    if (_activePatternIndex < 0)
        return;

    destroyPreview();

    const auto& entry = _patterns[_activePatternIndex];

    _previewObject = getOwner()->getScene()->createGameObject();
    _previewObject->setName("PatternPreview");

    _previewHexes.clear();

    for (const auto& hex : entry.pattern.getHexes()) {

        auto prefab = _engine->getAssetManager()->get<nlohmann::json>("prefabs/battle_hex.prefab");

        auto* hexObject = dzemikk::PrefabSerializer::instantiate(
            *getOwner()->getScene(), *prefab.get(), _engine->getAssetManager());

        hexObject->setParent(_previewObject);
        hexObject->transform()->setPosition(axialToWorld(hex, 1.0f));

        _previewHexes.push_back(hexObject);
    }
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

void game::PlayerPatternComponent::destroyPreview() {
    if (_previewObject) {
        _previewObject->getScene()->destroyGameObject(_previewObject);
        _previewObject = nullptr;
    }

    _previewHexes.clear();
}

void game::PlayerPatternComponent::tryRemovePlacedPatternUnderCursor() {
    int windowWidth = 0;
    int windowHeight = 0;

    glfwGetWindowSize(_engine->getWindow()->nativeHandle(), &windowWidth, &windowHeight);

    auto* collider = _engine->getCollisions()->raycast(
        _engine->getRenderer()->getCameraSystem().getActiveSceneCamera(),
        _engine->getInput()->GetMousePosition(), windowWidth, windowHeight);

    if (!collider)
        return;

    auto* worldHex = collider->getOwner()->getComponent<game::WorldHex>();

    if (!worldHex || !worldHex->getHexCell())
        return;

    HexCoord clicked = worldHex->getHexCell()->getCoord();

    for (size_t i = 0; i < _placedPatterns.size(); ++i) {

        const auto& placed = _placedPatterns[i];

        for (const auto& offset : placed.pattern.getHexes()) {

            HexCoord occupied(placed.origin.q() + offset.r(), placed.origin.r() + offset.q());

            if (occupied == clicked) {
                removePlacedPattern(i);
                return;
            }
        }
    }
}

void game::PlayerPatternComponent::removePlacedPattern(size_t index) {
    if (index >= _placedPatterns.size())
        return;

    auto& placed = _placedPatterns[index];
    
    if (_playerPatternStats) {
        _playerPatternStats->registerRemoval(placed.pattern);
    }

    for (auto* obj : placed.objects) {
        if (obj && obj->getScene())
            obj->getScene()->destroyGameObject(obj);
    }

    _placedPatterns.erase(_placedPatterns.begin() + index);

    if (_activePatternIndex < 0) {
        destroyPreview();
    }
}

const std::vector<game::PlayerPatternComponent::PlacedPattern>&
game::PlayerPatternComponent::getPlacedPatterns() const {
    return _placedPatterns;
}