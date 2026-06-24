#include "totem/totemEntity.h"

#include "animation/animationclip.h"
#include "animation/animationstatemachine.h"
#include "animation/vectortrack.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/serialize/prefabSerializer.h"
#include "game.h"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/trigonometric.hpp"
#include "map/HexPattern.h"
#include "player/playerPatternComponent.h"
#include "ui/combatUIPanel.h"

#include <ecs/components/light/pointLight.h>
#include <ecs/gameobject.h>
#include <ecs/scene.h>

void game::TotemEntity::onEnter(HexCellPtr cell) {
    if (!cell) {
        return;
    }

    if (getCell()) {
        onExit();
    }

    setCell(cell);

    cell->setEntity(this);
    cell->setState(HexCell::State::Totem);
}

void game::TotemEntity::onExit() {
    if (!getCell()) {
        return;
    }

    getCell()->setEntity(nullptr);
    getCell()->setState(HexCell::State::Empty);

    setCell(nullptr);
}

void game::TotemEntity::use() {
    if (_config.used) {
        return;
    }

    _config.used = true;

    auto* playerGO = _game->getCurrentScene().get()->findGameObjectByName("Player");
    auto* patternComponent = playerGO->getComponent<game::PlayerPatternComponent>();
    for (auto& pattern : _config.patterns) {
        patternComponent->addPattern(pattern, 3, 3);
    }

    auto* pattern = patternComponent->getPattern(patternComponent->getPatternCount() - 1);

    auto* playerPanel = this->getOwner()->getScene()->findGameObjectByName("Player_Panel");
    auto* combatPlayerPanel = playerPanel->getComponent<game::CombatUIPanel>();
    combatPlayerPanel->refresh();

    auto* hg = _game->getHexGrid();
    for (const auto& [uuid, bpi] : hg->getBlockingPatterns()) {
        if (patternComponent->hasPattern(bpi.pattern)) {
            hg->unlockChunk(bpi.blockedChunkId);
        }
    }

    animatePatterns();

    lightOff();
}

void game::TotemEntity::unuse() {
    if (!_config.used) {
        return;
    }
    _config.used = false;
    lightOn();
}

void game::TotemEntity::lightOff() {
    auto* lightGO = this->getOwner()->findChildByName("Light");
    auto* lightComp = lightGO->getComponent<dzemikk::PointLight>();
    lightComp->enabled(false);
}

void game::TotemEntity::lightOn() {
    auto* lightGO = this->getOwner()->findChildByName("Light");
    auto* lightComp = lightGO->getComponent<dzemikk::PointLight>();
    lightComp->enabled(true);
}

void game::TotemEntity::animatePatterns() {
    if (_config.patterns.empty()) {
        return;
    }

    // _mats[HexPattern::Type::ATK] = std::shared_ptr<dzemikk::Material>();
    // _mats[HexPattern::Type::ATK]->setShader(
    //     _game->getEngine()->getAssetManager()->get<dzemikk::Shader>("shaders/tile1"));
    // _mats[HexPattern::Type::ATK] = std::shared_ptr<dzemikk::Material>();
    // _mats[HexPattern::Type::ATK]->setShader(
    //     _game->getEngine()->getAssetManager()->get<dzemikk::Shader>("shaders/tile1"));
    // _mats[HexPattern::Type::ATK] = std::shared_ptr<dzemikk::Material>();
    // _mats[HexPattern::Type::ATK]->setShader(
    //     _game->getEngine()->getAssetManager()->get<dzemikk::Shader>("shaders/tile1"));

    _nextPatternIndex = 0;
    _patternSpawnTimer = 0.0F;
    _spawningPatterns = true;

    spawnNextPattern();
}

void game::TotemEntity::update(double deltaTime) {
    if (!_spawningPatterns) {
        return;
    }

    _patternSpawnTimer += static_cast<float>(deltaTime);
    if (_patternSpawnTimer >= 1.0F) {
        _patternSpawnTimer = 0.0F;
        spawnNextPattern();
    }
}

void game::TotemEntity::spawnNextPattern() {
    if (_nextPatternIndex >= _config.patterns.size()) {
        _spawningPatterns = false;
        return;
    }

    auto* assetManager = _game->getEngine()->getAssetManager();
    auto hexPrefab = assetManager->get<nlohmann::json>("prefabs/battle_hex.prefab");
    dzemikk::Scene* scene = _game->getCurrentScene().get();

    const auto& pattern = _config.patterns[_nextPatternIndex];
    _nextPatternIndex++;

    auto* patternGO = scene->createGameObject("Pattern");
    patternGO->transform()->setPosition(
        getCell()->getCoord().toWorldPosition(1.0F, 0.1F, getCell()->getHeight()));
    patternGO->transform()->setRotation(
        glm::angleAxis(glm::radians(90.0F), glm::vec3(0.0F, 0.0F, 1.0F)));

    auto* anim = patternGO->addComponent<dzemikk::Animator>();
    auto clip = std::make_shared<dzemikk::AnimationClip>(10.0F, 2.0F);
    clip->setLoop(false);
    _patternClips.push_back(clip);
    dzemikk::VectorTrack* posTrack = clip->addVectorTrack();
    dzemikk::VectorTrack* scaleTrack = clip->addVectorTrack();
    posTrack->bindPosition(*patternGO->transform());
    posTrack->addKey(
        {.time = 0.0F,
         .value = patternGO->transform()->getPosition() + glm::vec3(0.0F, 2.0F, 0.0F)});
    posTrack->addKey(
        {.time = 10.0F,
         .value = patternGO->transform()->getPosition() + glm::vec3(0.0F, 20.0F, 0.0F)});
    scaleTrack->bindScale(*patternGO->transform());
    scaleTrack->addKey({.time = 0.0F, .value = glm::vec3(0.0F)});
    scaleTrack->addKey({.time = 1.0F, .value = glm::vec3(0.5F)});
    scaleTrack->addKey({.time = 9.0F, .value = glm::vec3(0.5F)});
    scaleTrack->addKey({.time = 10.0F, .value = glm::vec3(0.0F)});
    auto sm = std::make_shared<dzemikk::AnimationStateMachine>();
    dzemikk::AnimationState* state = sm->addState("Move");
    state->setClip(clip.get());
    anim->setStateMachine(sm);
    anim->play("Move");

    for (const auto& coord : pattern.getHexes()) {
        auto* hexGO = dzemikk::PrefabSerializer::instantiate(*scene, *hexPrefab.get(), assetManager,
                                                             patternGO);
        hexGO->transform()->setPosition(coord.toWorldPosition(1.0F, 0.1F, 0.0F));

        auto color = glm::vec3(0.0F);
        switch (pattern.getType()) {
        case HexPattern::Type::ATK: {
            color = {0.8F, 0.0F, 0.0F};
            break;
        }
        case HexPattern::Type::DEF: {
            color = {0.0F, 0.0F, 0.8F};
            break;
        }
        case HexPattern::Type::HEAL: {
            color = {0.0F, 0.8F, 0.0F};
            break;
        }
        default: {
            color = glm::vec3(0.0F);
        }
        }

        hexGO->getComponent<dzemikk::MeshRenderer>()->getMaterial(0)->setAlbedoColor(color);
    }
}
