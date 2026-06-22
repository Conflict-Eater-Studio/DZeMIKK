#include "stateMachine/combatState.h"

#include "camera/cameraController.h"
#include "ecs/components/animator.h"
#include "enemySystem/combatArenaBuilder.h"
#include "enemySystem/enemyEntity.h"
#include "enemySystem/enemyManager.h"
#include "enemySystem/enemyPlanner.h"
#include "game.h"
#include "gameStateMachine.h"
#include "healthSystem.h"
#include "map/HexCell.h"
#include "map/HexChunk.h"
#include "map/HexGrid.h"
#include "player/playerMovement.h"
#include "player/playerPatternComponent.h"
#include "stateMachine/combatResolver.h"
#include "stateMachine/explorationState.h"
#include "ui/combatUIPanel.h"

#include <assetManager/assetHandle.h>
#include <assetManager/soundHandler.h>
#include <audio/audioManager.h>
#include <ecs/components/meshRenderer.h>
#include <ecs/components/transform.h>
#include <ecs/components/ui/imageRenderer.h>
#include <ecs/gameobject.h>
#include <ecs/scene.h>
#include <enemySystem/enemyPatternComponent.h>
#include <iostream>
#include <renderer/shader.h>

namespace combatSound {
FMOD::Channel* combatFMODChannel = nullptr;

struct SoundInitContext {
    dzemikk::AudioManager* audioManager;
};

void onMusicLoad(const dzemikk::AssetHandle<dzemikk::Sound>& sound, SoundInitContext& ctx) {
    combatFMODChannel =
        ctx.audioManager->play(*sound.get(), dzemikk::AudioManager::SoundType::Music, true);
    ctx.audioManager->getMusicGroup()->setVolume(0.1F);
}

void onSFXLoad(const dzemikk::AssetHandle<dzemikk::Sound>& sound, SoundInitContext& ctx) {
    ctx.audioManager->play(*sound.get(), dzemikk::AudioManager::SoundType::SFX, false);
    ctx.audioManager->getSFXGroup()->setVolume(0.5F);
}
} // namespace combatSound

void game::CombatState::onEnter() {
    _game->resetExplorationInputState();

    combatSound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
    dzemikk::AssetManager::AssetTask<dzemikk::Sound, combatSound::SoundInitContext> taskS;
    taskS.context = sCtx;
    taskS.onLoad = combatSound::onMusicLoad;
    _game->getEngine()->getAssetManager()->getAsync("audio/wartwa_na_czas_walki.wav", taskS);

    _phase = CombatPhase::PreparingBoard;

    _game->getCameraController()->setMode(CameraController::Mode::Combat);
    _game->enableCombatUI(true);


    initializeCombat();

    if (!_arenaCenterCell) {
        return;
    }

    setupInput();
    setupEnemyHealth();

    collectAnimatedHexes();

    _boardTransition = 0.0F;
    _enterAnimation = true;

    dzemikk::AssetManager::AssetTask<dzemikk::Sound, combatSound::SoundInitContext> taskS2;
    taskS2.context = sCtx;
    taskS2.onLoad = combatSound::onSFXLoad;
    _game->getEngine()->getAssetManager()->getAsync("audio/prime_wznoszeniePol.wav", taskS2);

    auto shader = _game->getEngine()->getAssetManager()->get<dzemikk::Shader>("shaders/tile1");
    auto material = std::make_shared<dzemikk::Material>();
    material->setShader(shader);
    material->setAlbedoTexture(_game->getEngine()->getAssetManager()->get<dzemikk::Texture>(
        "models/assets/hexy/hex_wypukly/hex_wypukly_BaseColor.png"));
    material->setEmissiveTexture(_game->getEngine()->getAssetManager()->get<dzemikk::Texture>(
        "models/assets/hexy/hex_wypukly/hex_wypukly_Emissive.png"));
    material->setMetallicTexture(_game->getEngine()->getAssetManager()->get<dzemikk::Texture>(
        "models/assets/hexy/hex_wypukly/hex_wypukly_Metallic.png"));
    material->setRoughnessTexture(_game->getEngine()->getAssetManager()->get<dzemikk::Texture>(
        "models/assets/hexy/hex_wypukly/hex_wypukly_Roughness.png"));

    _hexMaterials[HexPattern::Type::ATK] = material;
    _hexMaterials[HexPattern::Type::ATK]->setAlbedoColor({1.0F, 0.0F, 0.0F});

    _hexMaterials[HexPattern::Type::DEF] = material->clone();
    _hexMaterials[HexPattern::Type::DEF]->setAlbedoColor({0.0F, 0.0F, 1.0F});

    _hexMaterials[HexPattern::Type::HEAL] = material->clone();
    _hexMaterials[HexPattern::Type::HEAL]->setAlbedoColor({0.0F, 1.0F, 0.0F});

    _hexMaterials[HexPattern::Type::NONE] = material->clone();
    _hexMaterials[HexPattern::Type::NONE]->setAlbedoColor({0.3F, 0.3F, 0.3F});

    _enemyBattleHexMaterial = material->clone();
    _enemyBattleHexMaterial->setAlbedoColor({0.1F, 0.0F, 0.4F});

    _emptyEnemyBattleHexMaterial = material->clone();
    _emptyEnemyBattleHexMaterial->setAlbedoColor({1.0F, 1.0F, 1.0F});
    _emptyEnemyBattleHexMaterial->setAlbedoTexture(
        _game->getEngine()->getAssetManager()->get<dzemikk::Texture>("models/assets/hexy/hex_wypukly/hex_wypukly_BaseColor_unactive.png"));

    _showedPatternMaterial = material->clone();
    _showedPatternMaterial->setAlbedoColor({0.25F, 0.25F, 0.25F});

    auto* enemyAvatarGO = _game->getCurrentScene()
                              .get()
                              ->findGameObjectByName("Enemy_Avatar_Panel")
                              ->findDescendantByName("Avatar");
    auto enemyAvatarRenderer = enemyAvatarGO->getComponent<dzemikk::ImageRenderer>();

    switch (_currentEnemy->getEnemyPersonality()) {
    case EnemyPersonality::Aggressive:
        enemyAvatarRenderer->setTexture(
            _game->getEngine()->getAssetManager()->get<dzemikk::Texture>(
                "textures/ui grafiki/avatary/avatar4.png"));
        break;
    case EnemyPersonality::Defensive:
        enemyAvatarRenderer->setTexture(
            _game->getEngine()->getAssetManager()->get<dzemikk::Texture>(
                "textures/ui grafiki/avatary/avatar2.png"));
        break;
    case EnemyPersonality::Balanced:
        enemyAvatarRenderer->setTexture(
            _game->getEngine()->getAssetManager()->get<dzemikk::Texture>(
                "textures/ui grafiki/avatary/avatar3.png"));
        break;
    default:
        enemyAvatarRenderer->setTexture(
            _game->getEngine()->getAssetManager()->get<dzemikk::Texture>(
                "textures/ui grafiki/avatary/avatar4.png"));
        break;
    }

    startNewTurn();
}

void game::CombatState::onExit() {
    _game->enableCombatUI(false);

    auto scene = _game->getCurrentScene();

    auto* grid = _game->getHexGrid();

    if (!grid || !_currentEnemy || !_player) {
        return;
    }

    auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");
    auto* world = worldGO->getComponent<World>();

    auto player = world->getHexTransformByCell(*_player->getCell().get());
    for (auto g : player->getOwner()->getChildren()) {
        g->enabled(false);
    }

    for (auto* cell : _player->getTerritory()) {

        if (!cell) {
            continue;
        }

        cell->setType(HexCell::Type::Normal);

        auto transform = world->getHexTransformByCell(*cell);

        for (auto g : transform->getOwner()->getChildren()) {
            g->enabled(false);
        }

        cell->setDirty(true);
    }

    _player->clearTerritory();
    _playerPatternComponent->clearPlacedPatterns();
    _playerPatternComponent->deactivatePattern();

    const auto& enemyTerritory = _currentEnemy->getTerritory();

    for (auto* cell : enemyTerritory) {
        if (!cell) {
            continue;
        }

        if (!_playerDied) {
            cell->setType(HexCell::Type::Normal);
        }
        cell->setDirty(true);
    }

    if (auto enemyCell = grid->findCellByEntity(_currentEnemy); enemyCell) {
        auto* playerMovement = _player->getOwner()->getComponent<PlayerMovement>();

        if (playerMovement) {
            playerMovement->stopMovement();
        }

        if (!_playerDied) {
            _player->teleportTo(enemyCell);
        }
    }

    if (auto* em = scene.get()->findGameObjectByTag("EnemyManager")->getComponent<EnemyManager>();
        em && !_playerDied) {
        em->removeEnemy(_currentEnemy);
        auto* enemyGO = _currentEnemy->getOwner();
        _game->getCurrentScene().get()->destroyGameObject(enemyGO);
    }

    _currentEnemy = nullptr;
    _game->getEngine()->getInput()->OnKeyPressed.removeListener(_endTurnListenerId);

    dzemikk::UIActionRegistry::get().unregisterAction("Confirm_Round");

    _game->getEngine()->getAudioManager()->stop(combatSound::combatFMODChannel);

    if (_playerDied) {
        _currentEnemy = nullptr;
        _game->getEngine()->getInput()->OnKeyPressed.removeListener(_endTurnListenerId);
        dzemikk::UIActionRegistry::get().unregisterAction("Confirm_Round");
        _game->getEngine()->getAudioManager()->stop(combatSound::combatFMODChannel);
        return;
    }
}

void game::CombatState::onUpdate(float dt) {
    if (_exitAnimation) {

        _boardTransition -= dt * _animationExitSpeed;

        if (_boardTransition <= 0.0F) {

            _boardTransition = 0.0F;
            _exitAnimation = false;

            if (_playerDied) {
                _game->markPendingRestart();
                _game->setExplorationState();
            } else {

                if (_currentEnemy->getEnemyType() == EnemyType::Boss) {
                    _game->setCinematicState();
                } else {
                    _game->setExplorationState();
                }
            }
            return;
        }

        updateBoardVisibility(_boardTransition, true);
        return;
    }

    if (_enterAnimation) {
        _boardTransition += dt * _animationEnterSpeed;

        if (_boardTransition >= 1.0F) {
            _boardTransition = 1.0F;
            _enterAnimation = false;
        }

        updateBoardVisibility(_boardTransition, false);
    }

    if (_phase == CombatPhase::ResolveTurn) {

        _resultTimer -= dt;

        if (_resultTimer <= 0.0F) {
            startNewTurn();
        }
    }
}

void game::CombatState::startNewTurn() {
    _roundCount++;
    auto* textGO = _game->getCurrentScene()
                       .get()
                       ->findGameObjectByName("Round_Number")
                       ->findChildByName("Text");
    auto* textRenderer = textGO->getComponent<dzemikk::UITextRenderer>();
    std::string textFill = "ROUND " + std::to_string(_roundCount);
    textRenderer->text = textFill;

    _phase = CombatPhase::EnemyPlanning;

    _playerPatternComponent->clearPlacedPatterns();
    _playerPatternComponent->deactivatePattern();

    auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");
    auto* world = worldGO->getComponent<World>();

    for (auto* cell : _currentEnemy->getTerritory()) {
        auto* transform = world->getHexTransformByCell(*cell);
        if (!transform) {
            continue;
        }

        auto* mesh = transform->getOwner()->getComponent<dzemikk::MeshRenderer>();
        if (!mesh) {
            continue;
        }

        mesh->setMaterial(0, _enemyBattleHexMaterial);
    }

    auto* enemyManagerGO = _game->getCurrentScene().get()->findGameObjectByName("EnemyManager");
    auto* patternComponent = enemyManagerGO->getComponent<EnemyPatternComponent>();
    patternComponent->clearUsage();

    _revealedPatterns.clear();
    _revealedCells.clear();

    EnemyPlanner planner;

    _plannedPatterns.clear();
    _plannedPatterns =
        planner.planTurn(_game, _currentEnemy, patternComponent, _game->getHexGrid(), 0.75F,
                         _playerPatternComponent->getPlayerPatternStatsComponent());

    const auto& usage = patternComponent->getPatternUsage();

    std::unordered_set<HexCell*> usedCells;

    for (const auto& pattern : _plannedPatterns) {
        for (auto* cell : pattern.cells) {
            if (cell) {
                usedCells.insert(cell);
            }
        }
    }

    for (auto* cell : _currentEnemy->getTerritory()) {

        if (!cell) {
            continue;
        }

        if (usedCells.contains(cell)) {
            continue;
        }

        auto* transform = world->getHexTransformByCell(*cell);
        if (!transform) {
            continue;
        }

        auto* mesh = transform->getOwner()->getComponent<dzemikk::MeshRenderer>();
        if (!mesh) {
            continue;
        }

        mesh->setMaterial(0, _emptyEnemyBattleHexMaterial);
    }

    auto* enemyPanel = _game->getCurrentScene().get()->findGameObjectByName("Enemy_Panel");
    auto* enemyPanelUI = enemyPanel->getComponent<CombatUIPanel>();
    enemyPanelUI->refresh(true);

    _playerPatternComponent->refillAllPatterns();

    auto* playerPanel = _game->getCurrentScene().get()->findGameObjectByName("Player_Panel");
    auto* combatPlayerPanel = playerPanel->getComponent<game::CombatUIPanel>();
    combatPlayerPanel->refreshCounts();

    _phase = CombatPhase::PlayerTurn;
    _playerPatternComponent->setInteractionEnabled(true);
}

void game::CombatState::endPlayerTurn() {
    _playerPatternComponent->setInteractionEnabled(false);
    _phase = CombatPhase::ResolveTurn;

    resolveConflict();

    showEnemyPlannedPatterns();

    combatSound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
    dzemikk::AssetManager::AssetTask<dzemikk::Sound, combatSound::SoundInitContext> taskS;
    taskS.context = sCtx;
    taskS.onLoad = combatSound::onSFXLoad;
    _game->getEngine()->getAssetManager()->getAsync("audio/prime_zakonczenie_tury.wav", taskS);

    _resultTimer = 2.0F;
}

std::shared_ptr<dzemikk::Material> game::CombatState::getPatternMaterial(HexPattern::Type type) {
    switch (type) {
    case HexPattern::Type::ATK:
        return _hexMaterials[HexPattern::Type::ATK];

    case HexPattern::Type::DEF:
        return _hexMaterials[HexPattern::Type::DEF];

    case HexPattern::Type::HEAL:
        return _hexMaterials[HexPattern::Type::HEAL];

    default:
        return _hexMaterials[HexPattern::Type::NONE];
    }
}

void game::CombatState::showEnemyPlannedPatterns() {

    auto* worldGO = _game->getCurrentScene().get()->findGameObjectByName("World");

    auto* world = worldGO->getComponent<World>();

    for (const auto& pattern : _plannedPatterns) {

        for (auto* cell : pattern.cells) {

            if (!cell) {
                continue;
            }

            const auto& coord = cell->getCoord();

            auto* transform = world->getHexTransformByCell(*cell);

            if (!transform) {
                continue;
            }

            auto* mesh = transform->getOwner()->getComponent<dzemikk::MeshRenderer>();

            if (!mesh) {
                continue;
            }

            mesh->setMaterial(0, getPatternMaterial(pattern.type));
        }
    }
}

void game::CombatState::resolveConflict() {
    auto result = CombatResolver::resolve(*_playerPatternComponent, _plannedPatterns,
                                          _currentEnemy->getCell()->getCoord(),
                                          _player->getCell()->getCoord());

    auto* playerHealth = _game->getCurrentScene()
                             .get()
                             ->findGameObjectByName("Player_Avatar_Panel")
                             ->findDescendantByName("Health_Holder")
                             ->getComponent<HealthSystem>();

    if (playerHealth) {
        playerHealth->damage(result.damageToPlayer);
        playerHealth->heal(result.healToPlayer);
    }

    auto* enemyHealth = _game->getCurrentScene()
                            .get()
                            ->findGameObjectByName("Enemy_Avatar_Panel")
                            ->findDescendantByName("Health_Holder")
                            ->getComponent<HealthSystem>();

    if (enemyHealth) {
        enemyHealth->damage(result.damageToEnemy);
        enemyHealth->heal(result.healToEnemy);
    }

    if (enemyHealth->isDead()) {
        auto* grid = _game->getCurrentScene()
                         .get()
                         ->findGameObjectByTag("World")
                         ->getComponent<World>()
                         ->getGrid();

        auto enemyChunkId =
            grid->findChunkForCoord(_currentEnemy->getCell()->getCoord())->getPersistantId();
        auto enemyConfig = _currentEnemy->getConfig();
        auto enemyId = _currentEnemy->getId();
#if DZEMIKK_DEV_TOOLS
        spdlog::info("[CombatState] Enemy defeated. ID: {}, Chunk: {}",
                     boost::uuids::to_string(enemyId), boost::uuids::to_string(enemyChunkId));
#endif
        for (const auto& childChunk : enemyConfig.blocksChunks) {
            HexGrid::BridgeId bridgeId{enemyChunkId, childChunk};
#if DZEMIKK_DEV_TOOLS
            bool bridgeExists = grid->getBridges().contains(bridgeId);
            spdlog::info("[CombatState] Unlocking bridge {{ {}, {} }} - exists: {}",
                         boost::uuids::to_string(enemyChunkId), boost::uuids::to_string(childChunk),
                         bridgeExists);
            if (bridgeExists) {
                const auto& bridge = grid->getBridges().at(bridgeId);
                spdlog::info("[CombatState] Bridge has {} blocking enemies, {} hexes",
                             bridge.blockingEnemies.size(), bridge.hexes.size());
            }
#endif
            grid->unlockBridge(bridgeId, enemyId);
        }

        auto* enemyManagerGO = _game->getCurrentScene().get()->findGameObjectByName("EnemyManager");
        if (enemyManagerGO) {
            auto* enemyManager = enemyManagerGO->getComponent<EnemyManager>();
            if (enemyManager) {
                enemyManager->removeEnemy(_currentEnemy);
            }
        }
    }

    if (playerHealth->isDead()) {
        _exitAnimation = true;
        _boardTransition = 1.0F;
        _playerDied = true;

        combatSound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
        dzemikk::AssetManager::AssetTask<dzemikk::Sound, combatSound::SoundInitContext> taskS;
        taskS.context = sCtx;
        taskS.onLoad = combatSound::onSFXLoad;
        _game->getEngine()->getAssetManager()->getAsync("audio/prime_przegrana_walka_enhanced.wav",
                                                        taskS);

        dzemikk::AssetManager::AssetTask<dzemikk::Sound, combatSound::SoundInitContext> taskS2;
        taskS2.context = sCtx;
        taskS2.onLoad = combatSound::onSFXLoad;
        _game->getEngine()->getAssetManager()->getAsync("audio/prime_wznoszeniePol.wav", taskS2);
    }

    if (enemyHealth->isDead()) {
        _exitAnimation = true;
        _boardTransition = 1.0F;

        combatSound::SoundInitContext sCtx(_game->getEngine()->getAudioManager());
        dzemikk::AssetManager::AssetTask<dzemikk::Sound, combatSound::SoundInitContext> taskS;
        taskS.context = sCtx;
        taskS.onLoad = combatSound::onSFXLoad;
        _game->getEngine()->getAssetManager()->getAsync("audio/prime_gra_WYgrana.wav", taskS);

        dzemikk::AssetManager::AssetTask<dzemikk::Sound, combatSound::SoundInitContext> taskS2;
        taskS2.context = sCtx;
        taskS2.onLoad = combatSound::onSFXLoad;
        _game->getEngine()->getAssetManager()->getAsync("audio/prime_wznoszeniePol.wav", taskS2);
    }
}

void game::CombatState::updateBoardVisibility(float factor, bool exiting) {
    constexpr float DelayPerRing = 0.05F;

    for (auto& hex : _hiddenHexes) {

        float delay = (float)hex.distance * DelayPerRing;

        float localFactor = 0.0F;

        if (!exiting) {
            localFactor = (factor - delay) / (1.0F - delay);

        } else {

            localFactor = ((1.0F - factor) - delay) / (1.0F - delay);
            localFactor = 1.0F - localFactor;
        }

        localFactor = glm::clamp(localFactor, 0.0F, 1.0F);

        auto pos = hex.transform->getPosition();

        pos.y = glm::mix(hex.startY, hex.startY + _hiddenOffsetY, localFactor);

        hex.transform->setPosition(pos);
    }
}

void game::CombatState::initializeCombat() {
    auto* scene = _game->getCurrentScene().get();

    auto* playerGO = scene->findGameObjectByName("Player");
    _player = playerGO->getComponent<PlayerEntity>();

    auto* worldGO = scene->findGameObjectByName("World");
    auto* world = worldGO->getComponent<World>();

    auto* enemyManagerGO = scene->findGameObjectByName("EnemyManager");
    auto* enemyManager = enemyManagerGO->getComponent<EnemyManager>();

    auto playerCell = _game->getHexGrid()->findCellByEntity(_player);

    _currentEnemy = enemyManager->getEnemyByCell(playerCell.get());

    _player->getOwner()->transform()->setEulerAngles(glm::vec3(0.0F, -30.0F, 0.0F));
    _currentEnemy->getOwner()->transform()->setEulerAngles(glm::vec3(0.0F, -210.0F, 0.0F));

    auto arena = CombatArenaBuilder::build(_currentEnemy, _player, _game->getHexGrid(), world);

    _arenaCenterCell = arena.centerCell.get();

    if (!_arenaCenterCell) {
        return;
    }

    if (auto* movement = _player->getOwner()->getComponent<PlayerMovement>()) {

        movement->stopMovement();
    }

    _player->teleportTo(arena.centerCell);

    auto playerT = world->getHexTransformByCell(*_player->getCell().get());
    for (auto g : playerT->getOwner()->getChildren()) {
        g->enabled(false);
    }

    _playerPatternComponent = playerGO->getComponent<PlayerPatternComponent>();
    _playerPatternComponent->setEnemyEntity(_currentEnemy);
}

void game::CombatState::setupInput() {
    _endTurnListenerId = _game->getEngine()->getInput()->OnKeyPressed.addListener(
        [this](dzemikk::KeyPressedEvent& e) {
            if (e.GetKeyCode() == GLFW_KEY_1) {
                std::cout << "ShowPattern";
                revealRandomEnemyPattern();
                return;
            }

            if (e.GetKeyCode() == GLFW_KEY_2) {
                std::cout << "ShowPattern";
                revealRandomEnemyCell();
                return;
            }

            if (e.GetKeyCode() != GLFW_KEY_SPACE) {
                return;
            }

            if (_phase == CombatPhase::PlayerTurn) {
                endPlayerTurn();
            }
        });

    dzemikk::UIActionRegistry::get().registerAction(
        [this](const dzemikk::UIEvent&) {
            if (_phase == CombatPhase::PlayerTurn) {
                endPlayerTurn();
            }
        },
        "Confirm_Round");
}

void game::CombatState::setupEnemyHealth() {
    auto* enemyHealthGO = _game->getCurrentScene()
                              .get()
                              ->findGameObjectByName("Enemy_Avatar_Panel")
                              ->findDescendantByName("Health_Holder");

    auto* enemyHealthSystem = enemyHealthGO->getComponent<HealthSystem>();

    enemyHealthSystem->setMaxHealth(static_cast<float>(_currentEnemy->getHp()), true);
}

void game::CombatState::collectAnimatedHexes() {

    _hiddenHexes.clear();

    auto* world =
        _game->getCurrentScene().get()->findGameObjectByName("World")->getComponent<World>();

    const HexCoord centerCoord = _arenaCenterCell->getCoord();

    int visibleRadius = 18;

    auto playerCellPtr = _player->getCell();
    auto enemyCellPtr = _currentEnemy->getCell();

    for (const auto& chunk : _game->getHexGrid()->getChunks()) {

        for (const auto& [coord, cellPtr] : chunk.second->getHexes()) {

            auto* cell = cellPtr.get();

            if (!shouldAnimateCell(cell, centerCoord, visibleRadius, playerCellPtr.get(),
                                   enemyCellPtr.get())) {
                continue;
            }

            addCellToAnimation(cell, world, centerCoord);
        }
    }
}

bool game::CombatState::shouldAnimateCell(HexCell* cell, const HexCoord& centerCoord,
                                          int visibleRadius, HexCell* playerCell,
                                          HexCell* enemyCell) const {
    if (!cell) {
        return false;
    }

    if (HexCoord::distance(cell->getCoord(), centerCoord) > visibleRadius) {
        return false;
    }

    if (_player->getTerritory().contains(cell)) {
        return false;
    }

    if (_currentEnemy->getTerritory().contains(cell)) {
        return false;
    }

    if (cell == playerCell || cell == enemyCell) {
        return false;
    }

    return true;
}

void game::CombatState::addCellToAnimation(HexCell* cell, World* world,
                                           const HexCoord& centerCoord) {
    int distance = HexCoord::distance(cell->getCoord(), centerCoord);

    if (auto* transform = world->getHexTransformByCell(*cell)) {

        _hiddenHexes.push_back({transform, transform->getPosition().y, distance});
    }

    auto* entity = cell->getEntity();

    if (!entity || !entity->getOwner()) {
        return;
    }

    auto* entityTransform = entity->getOwner()->getComponent<dzemikk::Transform>();

    if (!entityTransform) {
        return;
    }

    _hiddenHexes.push_back({entityTransform, entityTransform->getPosition().y, distance});
}

void game::CombatState::revealRandomEnemyPattern() {
    if (_phase != CombatPhase::PlayerTurn) {
        return;
    }

    std::vector<int> available;

    for (int i = 0; i < static_cast<int>(_plannedPatterns.size()); ++i) {
        if (!_revealedPatterns.contains(i)) {
            available.push_back(i);
        }
    }

    if (available.empty()) {
        return;
    }

    int randomIndex = available[rand() % available.size()];

    _revealedPatterns.insert(randomIndex);

    showPattern(randomIndex);
}

void game::CombatState::showPattern(int index) {

    auto* world =
        _game->getCurrentScene().get()->findGameObjectByName("World")->getComponent<World>();

    const auto& pattern = _plannedPatterns[index];

    for (auto* cell : pattern.cells) {

        if (!cell) {
            continue;
        }

        auto* transform = world->getHexTransformByCell(*cell);

        if (!transform) {
            continue;
        }

        auto* mesh = transform->getOwner()->getComponent<dzemikk::MeshRenderer>();

        if (mesh) {
            mesh->setMaterial(0, _showedPatternMaterial);
        }
    }
}

void game::CombatState::revealRandomEnemyCell() {
    if (_phase != CombatPhase::PlayerTurn) {
        return;
    }

    struct Candidate {
        HexCell* cell;
        HexPattern::Type type;
    };

    std::vector<Candidate> available;

    for (const auto& pattern : _plannedPatterns) {
        for (auto* cell : pattern.cells) {
            if (!cell) {
                continue;
            }

            if (_revealedCells.contains(cell)) {
                continue;
            }

            available.push_back({cell, pattern.type});
        }
    }

    if (available.empty()) {
        return;
    }

    const auto& revealed = available[rand() % available.size()];

    _revealedCells.insert(revealed.cell);

    showCellColor(revealed.cell, revealed.type);
}

void game::CombatState::showCellColor(HexCell* cell, HexPattern::Type type) {
    auto* world =
        _game->getCurrentScene().get()->findGameObjectByName("World")->getComponent<World>();

    auto* transform = world->getHexTransformByCell(*cell);

    if (!transform) {
        return;
    }

    auto* mesh = transform->getOwner()->getComponent<dzemikk::MeshRenderer>();

    if (!mesh) {
        return;
    }

    mesh->setMaterial(0, getPatternMaterial(type));
}

game::EnemyEntity* game::CombatState::getCurrentEnemy() const {
    return _currentEnemy;
}

void game::CombatState::removeHiddenHex(HexCell* cell) {
    auto* world =
        _game->getCurrentScene().get()->findGameObjectByName("World")->getComponent<World>();

    auto* transform = world->getHexTransformByCell(*cell);

    if (!transform) {
        return;
    }

    std::erase_if(_hiddenHexes,
                  [transform](const AnimatedHex& hex) { return hex.transform == transform; });
}
