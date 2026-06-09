#include "player/playerPatternComponent.h"
#include "player/playerPatternStatsComponent.h"
#include "enemySystem/enemyEntity.h"
#include "game.h"
#include "scripts/world/worldHex.h"
#include "gameStateMachine.h"
#include "stateMachine/combatState.h"

#include <iostream>

#include <core/engine.h>
#include <collisions/collisions.h>
#include <core/window.h>
#include <renderer/renderer.h>
#include <ecs/serialize/prefabSerializer.h>
#include <ecs/scene.h>
#include <assetManager/assetmanager.h>
#include <ecs/components/collider.h>
#include <ecs/components/meshRenderer.h>

namespace {
constexpr std::string_view BATTLE_HEX_PREFAB = "prefabs/battle_hex.prefab";
constexpr std::string_view PREVIEW_OBJECT_NAME = "PatternPreview";
}

void game::PlayerPatternComponent::start() {
    // atk1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::ATK, 1.0F), -1);
    addPattern(HexPattern({{0, 0}, {1, -1}}, HexPattern::Type::ATK, 1.1F), -1);
    //addPattern(HexPattern({{-1, 1}, {0, 0}, {1, -1}}, HexPattern::Type::ATK, 1.2F), -1);

    // atk2
    //addPattern(HexPattern({{-1, 0}, {0, 0}, {1, -1}}, HexPattern::Type::ATK, 1.2F), -1);

    // atk3
    //addPattern(HexPattern({{0, 0}, {1, -1}, {-1, 0}, {0, 1}}, HexPattern::Type::ATK, 1.3F), -1);

    // def1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::DEF), -1);
    addPattern(HexPattern({{0, 0}, {1, -1}}, HexPattern::Type::DEF, 1.1F), -1);

    // def2
    //addPattern(HexPattern({{-1, 1}, {0, 0}, {1, -1}}, HexPattern::Type::DEF, 1.2F), -1);

    // hp1
    addPattern(HexPattern({{0, 0}}, HexPattern::Type::HEAL), -1);
    addPattern(HexPattern({{0, 0}, {1, -1}}, HexPattern::Type::HEAL, 0.6F), -1);

    // bonus
    //addPattern(HexPattern({{0, 0}, {1, -1}}, HexPattern::Type::BONUSHEX, 1.0F), 1);

    _playerPatternStats = getOwner()->getComponent<PlayerPatternStatsComponent>();

    _onMousePressedListenerID = _engine->getInput()->OnMouseButtonPressed.addListener(
        [this](dzemikk::MouseButtonPressedEvent& e) { onMouseButtonPressed(e); });

    _rotatePatternListenerID = _engine->getInput()->OnMouseScrolled.addListener(
            [this](dzemikk::MouseScrolledEvent& e) { onMouseScrolled(e); });
}

void game::PlayerPatternComponent::update(double deltaTime) {
    if (!_interactionEnabled) {
        return;
    }

    if (_activePatternIndex < 0) {
        return;
    }

    if (!updatePreviewOrigin()) {
        return;
    }

    validateCurrentPattern();
    updatePreviewVisuals(_currentPreviewOriginCollider, getPatternPreviewColor());
}

void game::PlayerPatternComponent::onDestroy() {
    _engine->getInput()->OnMouseButtonPressed.removeListener(_onMousePressedListenerID);
    _engine->getInput()->OnMouseScrolled.removeListener(_rotatePatternListenerID);
}

bool game::PlayerPatternComponent::usePattern(size_t index) {
    if (!_interactionEnabled) {
        return false;
    }

    if (!canUsePattern(index)) {
        return false;
    }

    auto& entry = _patterns[index];

    if (entry.count > 0) {
        entry.count--;
    }

    _activePatternIndex = static_cast<int>(index);
    destroyPreview();
    createPreviewFromPattern(entry.pattern);

    return true;
}

void game::PlayerPatternComponent::setEngine(dzemikk::Engine* engine) {
    _engine = engine;
}

std::string game::PlayerPatternComponent::typeName() const {
    return "PlayerPatternComponent";
}

void game::PlayerPatternComponent::setInteractionEnabled(bool enabled) {
    _interactionEnabled = enabled;

    if (!enabled) {
        deactivatePattern();
    }
}

game::PlayerPatternStatsComponent* game::PlayerPatternComponent::getPlayerPatternStatsComponent() {
    return _playerPatternStats;
}

void game::PlayerPatternComponent::setPlayerEntity(game::PlayerEntity* playerEntity) {
    _playerEntity = playerEntity;
}

void game::PlayerPatternComponent::setEnemyEntity(game::EnemyEntity* enemyEntity) {
    _enemyEntity = enemyEntity;
}

void game::PlayerPatternComponent::setCombatBoardOffset(float offset) {
    _combatBoardOffset = offset;
}

bool game::PlayerPatternComponent::hasActivePattern() const {
    return _activePatternIndex >= 0;
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
    float x = hexSize * std::numbers::sqrt3_v<float> * ((float)coord.q() + (float)coord.r() * 0.5F);

    float z = hexSize * 1.5F * (float)coord.r();

    return {x, 0.0F, z};
}

bool game::PlayerPatternComponent::confirmPattern() {
    if (_activePatternIndex < 0 || !_currentPreviewValid) {
        return false;
    }

    const auto& pattern = _patterns[_activePatternIndex].pattern;

    if (pattern.getType() == HexPattern::Type::BONUSHEX) {
        confirmBonusHex(pattern);
        return true;
    }

    PlacedPattern placed(pattern, _currentPreviewOrigin);

    for (const auto& offset : pattern.getHexes()) {
        HexCoord worldCoord(_currentPreviewOrigin.q() + offset.r(),
                            _currentPreviewOrigin.r() + offset.q());

        placed.worldCells.push_back(worldCoord);
    }

    for (auto* object : _previewHexes) {
        if (object) {
            placed.objects.push_back(object);
        }
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

void game::PlayerPatternComponent::confirmBonusHex(const HexPattern& pattern) {
    for (const auto& offset : pattern.getHexes()) {

        HexCoord targetCoord(_currentPreviewOrigin.q() + offset.r(),
                             _currentPreviewOrigin.r() + offset.q());

        auto cell = _grid->getCell(targetCoord);

        if (!cell) {
            continue;
        }

        cell->setType(HexCell::Type::PlayerBattleHex);
        cell->setDirty(true);

        _playerEntity->addTerritoryCell(cell.get());

        auto* root = _playerEntity->getOwner()->getScene()->findGameObjectByName("Root");
        auto* stateMachine = root->getComponent<GameStateMachine>();
        auto* state = stateMachine->getCurrentStateAs<CombatState>();
        state->removeHiddenHex(cell.get());

        auto* world = _playerEntity->getOwner()
                          ->getScene()
                          ->findGameObjectByName("World")
                          ->getComponent<World>();
        if (auto* worldHex = world->getHexTransformByCell(*cell.get())) {
            glm::vec3 pos = worldHex->getPosition();
            pos.y -= _combatBoardOffset;
            worldHex->setPosition(pos);
        }
    }

    deactivatePattern();
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

            if (occupied == coord) {
                return true;
            }
        }
    }

    return false;
}

void game::PlayerPatternComponent::restartPreview() {
    if (_activePatternIndex < 0) {
        return;
    }

    destroyPreview();

    const auto& entry = _patterns[_activePatternIndex];
    createPreviewFromPattern(entry.pattern);
}

void game::PlayerPatternComponent::deactivatePattern() {
    _activePatternIndex = -1;
    destroyPreview();
}

void game::PlayerPatternComponent::destroyPreview() {
    if (_previewObject) {
        _previewObject->getScene()->destroyGameObject(_previewObject);
        _previewObject = nullptr;
    }

    _previewHexes.clear();
}

void game::PlayerPatternComponent::tryRemovePlacedPatternUnderCursor() {
    auto* worldHex = getWorldHexUnderCursor();

    if (!worldHex || !worldHex->getHexCell()) {
        return;
    }

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
    if (index >= _placedPatterns.size()) {
        return;
    }

    auto& placed = _placedPatterns[index];
    
    if (_playerPatternStats) {
        _playerPatternStats->registerRemoval(placed.pattern);
    }

    for (auto* obj : placed.objects) {
        if (obj && obj->getScene()) {
            obj->getScene()->destroyGameObject(obj);
        }
    }

    using Diff = decltype(_placedPatterns)::difference_type;

    _placedPatterns.erase(_placedPatterns.begin() + static_cast<Diff>(index));

    if (_activePatternIndex < 0) {
        destroyPreview();
    }
}

const std::vector<game::PlayerPatternComponent::PlacedPattern>&
game::PlayerPatternComponent::getPlacedPatterns() const {
    return _placedPatterns;
}

void game::PlayerPatternComponent::onMouseButtonPressed(dzemikk::MouseButtonPressedEvent& e) {
    const int button = e.GetMouseButton();

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        handleLeftClick();
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        handleRightClick();
    }
}

void game::PlayerPatternComponent::handleLeftClick() {
    if (!_interactionEnabled) {
        return;
    }

    confirmPattern();
}

void game::PlayerPatternComponent::handleRightClick() {
    if (!_interactionEnabled) {
        return;
    }
    
    if (_activePatternIndex >= 0) {
        deactivatePattern();
    }

    tryRemovePlacedPatternUnderCursor();
}

bool game::PlayerPatternComponent::updatePreviewOrigin() {
    auto* worldHex = getWorldHexUnderCursor();

    if (!worldHex || !worldHex->getHexCell()) {
        return false;
    }

    auto cell = worldHex->getHexCell();
    _currentPreviewOrigin = cell->getCoord();

    return true;
}

void game::PlayerPatternComponent::validateCurrentPattern() {

    const auto& pattern = _patterns[_activePatternIndex].pattern;

    if (pattern.getType() == HexPattern::Type::BONUSHEX) {
        validateBonusHexPattern(pattern);
        return;
    }

    validateCombatPattern(pattern);
}

void game::PlayerPatternComponent::validateCombatPattern(const HexPattern& pattern) {
    bool validPattern = true;

    for (const auto& offset : pattern.getHexes()) {

        HexCoord targetCoord(_currentPreviewOrigin.q() + offset.r(),
                             _currentPreviewOrigin.r() + offset.q());

        auto targetCell = _grid->getCell(targetCoord);

        if (!targetCell || targetCell->getType() != HexCell::Type::PlayerBattleHex ||
            isCellOccupiedByPattern(targetCoord)) {
            validPattern = false;
            break;
        }
    }

    _currentPreviewValid = validPattern;
}

void game::PlayerPatternComponent::validateBonusHexPattern(const HexPattern& pattern) {
    bool valid = true;
    bool touchesPlayerTerritory = false;

    std::vector<HexCell*> bonusCells;

    for (const auto& offset : pattern.getHexes()) {

        HexCoord targetCoord(_currentPreviewOrigin.q() + offset.r(),
                             _currentPreviewOrigin.r() + offset.q());

        auto targetCell = _grid->getCell(targetCoord);

        if (!targetCell) {
            valid = false;
            break;
        }

        if (isCellOccupiedByPattern(targetCoord)) {
            valid = false;
            break;
        }

        if (_playerEntity->getTerritory().contains(targetCell.get())) {
            valid = false;
            break;
        }

        if (_enemyEntity && _enemyEntity->getTerritory().contains(targetCell.get())) {
            valid = false;
            break;
        }

        bonusCells.push_back(targetCell.get());
    }

    if (!valid) {
        _currentPreviewValid = false;
        return;
    }

    for (auto* cell : bonusCells) {

        for (const auto& neighbourCoord : HexCoord::getNeighbors(cell->getCoord())) {
            auto neighbour = _grid->getCell(neighbourCoord);

            if (!neighbour) {
                continue;
            }

            if (_playerEntity->getTerritory().contains(neighbour.get())) {
                touchesPlayerTerritory = true;
                break;
            }
        }

        if (touchesPlayerTerritory) {
            break;
        }
    }

    _currentPreviewValid = touchesPlayerTerritory;
}

glm::vec4 game::PlayerPatternComponent::getPatternPreviewColor() const {
    glm::vec4 color;
    auto pattern = _patterns[_activePatternIndex].pattern;

    switch (pattern.getType()) {

    case HexPattern::Type::ATK:
        color = _currentPreviewValid ? glm::vec4(1.0F, 0.0F, 0.0F, 1.0F)
                                     : glm::vec4(1.0F, 0.7F, 0.7F, 1.0F);
        break;

    case HexPattern::Type::DEF:
        color = _currentPreviewValid ? glm::vec4(0.0F, 0.0F, 1.0F, 1.0F)
                                     : glm::vec4(0.7F, 0.7F, 1.0F, 1.0F);
        break;

    case HexPattern::Type::HEAL:
        color = _currentPreviewValid ? glm::vec4(0.0F, 1.0F, 0.0F, 1.0F)
                                     : glm::vec4(0.7F, 1.0F, 0.7F, 1.0F);
        break;

    case HexPattern::Type::BONUSHEX:
        color = _currentPreviewValid ? glm::vec4(1.0F, 0.84F, 0.0F, 1.0F)
                                     : glm::vec4(1.0F, 0.84F, 0.7F, 1.0F);
        break;
    default:
        color = glm::vec4(1.0F);
        break;
    }

    return color;
}

void game::PlayerPatternComponent::updatePreviewVisuals(dzemikk::Collider* collider,
                                                        const glm::vec4& color) {
    auto* transform = collider->getOwner()->transform();

    _previewObject->transform()->setPosition({transform->getPosition().x,
                                              transform->getPosition().y + 0.6F,
                                              transform->getPosition().z});

    for (auto* hex : _previewHexes) {
        auto* renderer = hex->getComponent<dzemikk::MeshRenderer>();

        if (renderer) {
            renderer->setColor(color);
            renderer->setCullingRadius(50.0F);
        }
    }
}

void game::PlayerPatternComponent::createPreviewFromPattern(const HexPattern& pattern) {
    auto previewPrefab = _engine->getAssetManager()->get<nlohmann::json>(BATTLE_HEX_PREFAB.data());

    _previewObject = getOwner()->getScene()->createGameObject();
    _previewObject->setName(PREVIEW_OBJECT_NAME.data());

    for (const auto& hex : pattern.getHexes()) {
        auto* hexObject = dzemikk::PrefabSerializer::instantiate(
            *getOwner()->getScene(), *previewPrefab.get(), _engine->getAssetManager());

        hexObject->setParent(_previewObject);

        hexObject->transform()->setPosition(axialToWorld(hex, 1.0F));

        _previewHexes.push_back(hexObject);
    }
}

game::WorldHex* game::PlayerPatternComponent::getWorldHexUnderCursor() {
    int windowWidth = 0;
    int windowHeight = 0;

    glfwGetWindowSize(_engine->getWindow()->nativeHandle(), &windowWidth, &windowHeight);

    dzemikk::Collider* collider = _engine->getCollisions()->raycast(
        _engine->getRenderer()->getCameraSystem().getActiveSceneCamera(), nullptr,
        _engine->getInput()->GetMousePosition(), (float)windowWidth, (float)windowHeight);

    if (!collider) {
        return nullptr;
    }

    _currentPreviewOriginCollider = collider;

    auto* worldHex = collider->getOwner()->getComponent<game::WorldHex>();

    return worldHex;
}

void game::PlayerPatternComponent::onMouseScrolled(dzemikk::MouseScrolledEvent& e) {
    if (!_interactionEnabled) {
        return;
    }

    if (_activePatternIndex < 0) {
        return;
    }

    auto& pattern = _patterns[_activePatternIndex].pattern;

    auto rotation = e.GetYOffset() > 0 ? HexPattern::Rotation::Clockwise
                                       : HexPattern::Rotation::CounterClockwise;

    pattern.rotate(rotation);

    destroyPreview();
    createPreviewFromPattern(pattern);
}