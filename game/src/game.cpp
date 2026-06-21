#pragma once

#include "game.h"

#include "assetManager/assetmanager.h"
#include "boost/uuid/string_generator.hpp"
#include "camera/cameraController.h"
#include "collisions/collisions.h"
#include "core/engine.h"
#include "core/time.h"
#include "core/window.h"
#include "dialog/dialogManager.h"
#include "ecs/components/camera.h"
#include "ecs/components/collider.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/ui/gridLayout.h"
#include "ecs/components/ui/iUIInteractable.h"
#include "ecs/components/ui/uiActionRegistry.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/scenemanager.h"
#include "ecs/serialize/prefabSerializer.h"
#include "enemySystem/enemyTypes.h"
#include "input/input.h"
#include "map/HexCoord.h"
#include "map/HexPattern.h"
#include "map/ItemEntity.h"
#include "map/PlayerEntity.h"
#include "player/inventory.h"
#include "player/playerMovement.h"
#include "renderer/cameraSystem.h"
#include "renderer/material.h"
#include "renderer/model.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "scripts/world/world.h"
#include "scripts/world/worldHex.h"
#include "utils/perlin.h"

#include <GLFW/glfw3.h>
#include <ecs/components/ui/imageRenderer.h>
#include <filesystem>
#include <memory>
#include <unordered_set>
#include <utility>

#if DZEMIKK_DEV_TOOLS
#include <imgui.h>
#endif
#include "animation/animationclip.h"
#include "ecs/components/antiAliasingEffect.h"
#include "ecs/components/colorGradingEffect.h"
#include "ecs/components/fxaaPostProcessEffect.h"
#include "ecs/components/outlinePostProcessEffect.h"
#include "ecs/components/postProcessEffect.h"
#include "ecs/components/ui/uiSlider.h"
#include "enemySystem/enemyManager.h"
#include "enemySystem/enemyPatternComponent.h"
#include "enemySystem/territoryPatternRegistry.h"
#include "item/itemManager.h"
#include "player/playerPatternComponent.h"
#include "player/playerPatternStatsComponent.h"
#include "scripts/world/worldVisualManager.h"
#include "stateMachine/cinematicState.h"
#include "stateMachine/combatState.h"
#include "stateMachine/explorationState.h"
#include "totem/totemDialogEntity.h"
#include "totem/totemEntity.h"
#include "totem/totemManager.h"
#include "ui/combatUIPanel.h"
#include "ui/logoComponent.h"
#include "ui/uIPulseEffect.h"

#include <animation/animationstatemachine.h>
#include <audio/audioManager.h>
#include <audio/sound.h>
#include <ecs/components/animator.h>
#include <ecs/components/light/pointLight.h>
#include <ecs/components/skinnedMeshRenderer.h>
#include <gameStateMachine.h>
#include <healthSystem.h>
#include <iostream>
#include <random>

static std::mt19937 rng{std::random_device{}()};

static float randFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

static glm::vec3 randVec3(float min, float max) {
    return glm::vec3(randFloat(min, max), randFloat(min, max), randFloat(min, max));
}

void Game::spawnRandomPointLight() {
    auto& reg = dzemikk::ComponentRegistry::get();

    auto go = _mainScene.get()->createGameObject();

    auto* light = go->addComponent<dzemikk::PointLight>();
    light->setColor(randVec3(0.2f, 1.0f));
    light->setIntensity(randFloat(0.5f, 5.0f));
    light->setRange(randFloat(5.0f, 25.0f));

    auto* t = go->transform();
    t->setPosition(randVec3(-20.0f, 20.0f));

    std::cout << "Spawned random point light\n";
}

void printHierarchy(dzemikk::GameObject* obj, int depth = 0) {
    if (!obj)
        return;

    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }

    std::cout << obj->getName() << "\n";

    for (auto* child : obj->getChildren()) {
        printHierarchy(child, depth + 1);
    }
}

struct SkyboxInitContext {
    dzemikk::AssetHandle<dzemikk::Shader> shader;
    dzemikk::Renderer* renderer{};
    dzemikk::Engine* engine{};
};

void onSkyboxLoad(const dzemikk::AssetHandle<dzemikk::Skybox>& skybox, SkyboxInitContext& ctx) {
    auto skyboxShader = ctx.engine->getAssetManager()->get<dzemikk::Shader>("shaders/skybox");
    skybox.get()->setShader(skyboxShader);
    ctx.renderer->setSkybox(skybox);
}

Game::Game(dzemikk::Engine* engine) : _engine(engine) {}

void Game::start() {
    startGame();
    _engine->start();
}

void Game::startGame() {
    auto* assetManager = _engine->getAssetManager();
    auto* sceneManager = _engine->getSceneManager();

    setupSkybox();

    _mainScene = assetManager->get<dzemikk::Scene>("scenes/gameplay8.json");
    _menuScene = assetManager->get<dzemikk::Scene>("scenes/menu3.json");
    _creditsScene = assetManager->get<dzemikk::Scene>("scenes/credits.json");

    std::shared_ptr<dzemikk::Scene> sceneShared(_mainScene.get(), [](dzemikk::Scene*) {});
    std::shared_ptr<dzemikk::Scene> menuShared(_menuScene.get(), [](dzemikk::Scene*) {});
    std::shared_ptr<dzemikk::Scene> creditsShared(_creditsScene.get(), [](dzemikk::Scene*) {});
    sceneManager->loadScene(sceneShared);
    sceneManager->loadScene(menuShared);
    sceneManager->loadScene(creditsShared);
    sceneManager->setActiveScene(menuShared);

    auto* btnResetInteractable = _menuScene.get()
                                     ->findGameObjectByName("ResetButton")
                                     ->getComponent<dzemikk::IUIInteractable>();
    if (!std::filesystem::exists("./world.json")) {
        btnResetInteractable->setInteractable(false);
    } else {
        btnResetInteractable->setInteractable(true);
    }

    auto logo = _menuScene.get()->findGameObjectByName("Logo");
    logo->addComponent<game::LogoComponent>();

    setupMainCamera();
    setupUICamera();
    setupWorld();
    setupPlayer();
    registerDefaultTerritories();
    setupEnemies();
    // Setup Items and Totems ALWAYS after Enemies
    setupItems();
    setupTotems();
    setupDialogs();

    // Setup visuals ALWAYS at the end
    setupWorldVisuals();

    if (_worldGO) {
        if (auto* world = _worldGO->getComponent<game::World>(); world) {
            world->saveToFile("./world.json");
        }
    }

    auto* root = _mainScene.get()->findGameObjectByName("Root");
    _stateMachine = root->addComponent<game::GameStateMachine>();

    _cameraController = _mainCamera->getOwner()->addComponent<game::CameraController>();
    _cameraController->setPlayerTransform(_playerEntity->getOwner()->transform());

    _stateMachine->setState(std::make_unique<game::ExplorationState>(this));

    dzemikk::UIActionRegistry::get().registerAction(
        [this](const dzemikk::UIEvent&) {
            _stateMachine->setState(std::make_unique<game::ExplorationState>(this));
        },
        "E");

    dzemikk::UIActionRegistry::get().registerAction(
        [this](const dzemikk::UIEvent&) {
            _stateMachine->setState(std::make_unique<game::CombatState>(this));
        },
        "C");

    dzemikk::UIActionRegistry::get().registerAction(
        [this](const dzemikk::UIEvent&) {
            if (auto* inventory = _playerGO->getComponent<game::Inventory>(); inventory) {
                inventory->tryUseItem(game::ItemEntity::ItemType::RevealPattern);
            }
        },
        "combat.reveal.randomPattern");
    dzemikk::UIActionRegistry::get().registerAction(
        [this](const dzemikk::UIEvent&) {
            if (auto* inventory = _playerGO->getComponent<game::Inventory>(); inventory) {
                inventory->tryUseItem(game::ItemEntity::ItemType::RevealHex);
            }
        },
        "combat.reveal.randomHex");

    dzemikk::UIActionRegistry::get().registerAction(
        [this](const dzemikk::UIEvent&) {
            if (_stateMachine->getCurrentStateAs<game::ExplorationState>()) {
                return;
            }

            auto tooltipsGO = _mainScene.get()->findGameObjectByName("Tooltips_Panel");
            auto itemTooltip = tooltipsGO->findDescendantByName("Item");

            auto iconGO = itemTooltip->findChildByName("Icon");
            auto iconRenderer = iconGO->getComponent<dzemikk::ImageRenderer>();
            iconRenderer->setTexture(_engine->getAssetManager()->get<dzemikk::Texture>(
                "textures/ui grafiki/avatary/obsydian.png"));

            auto nameGO = itemTooltip->findChildByName("Name");
            auto nameText = nameGO->getComponent<dzemikk::UITextRenderer>();
            nameText->text = "REVEAL PATTERN";

            auto textGO = itemTooltip->findChildByName("Text");
            auto text = textGO->getComponent<dzemikk::UITextRenderer>();
            text->text = "Reveals enemy pattern shape";

            itemTooltip->enabled(true);
        },
        "combat.enter.randomPattern");
    dzemikk::UIActionRegistry::get().registerAction(
        [this](const dzemikk::UIEvent&) {
            if (_stateMachine->getCurrentStateAs<game::ExplorationState>()) {
                return;
            }

            auto tooltipsGO = _mainScene.get()->findGameObjectByName("Tooltips_Panel");
            auto itemTooltip = tooltipsGO->findDescendantByName("Item");

            auto iconGO = itemTooltip->findChildByName("Icon");
            auto iconRenderer = iconGO->getComponent<dzemikk::ImageRenderer>();
            iconRenderer->setTexture(_engine->getAssetManager()->get<dzemikk::Texture>(
                "textures/ui grafiki/avatary/muszla.png"));

            auto nameGO = itemTooltip->findChildByName("Name");
            auto nameText = nameGO->getComponent<dzemikk::UITextRenderer>();
            nameText->text = "REVEAL HEX";

            auto textGO = itemTooltip->findChildByName("Text");
            auto text = textGO->getComponent<dzemikk::UITextRenderer>();
            text->text = "Reveals the color/type of 1 enemy hex";

            itemTooltip->enabled(true);
        },
        "combat.enter.randomHex");

    dzemikk::UIActionRegistry::get().registerAction(
        [this](const dzemikk::UIEvent&) {
            auto tooltipsGO = _mainScene.get()->findGameObjectByName("Tooltips_Panel");

            auto itemTooltip = tooltipsGO->findDescendantByName("Item");
            itemTooltip->enabled(false);
        },
        "combat.exit.randomPattern");
    dzemikk::UIActionRegistry::get().registerAction(
        [this](const dzemikk::UIEvent&) {
            auto tooltipsGO = _mainScene.get()->findGameObjectByName("Tooltips_Panel");

            auto itemTooltip = tooltipsGO->findDescendantByName("Item");
            itemTooltip->enabled(false);
        },
        "combat.exit.randomHex");

    dzemikk::UIActionRegistry::get().registerAction(
        [this, sceneManager, sceneShared](const dzemikk::UIEvent&) {
            _gameStarted = true;
            sceneManager->setActiveScene(sceneShared);
        },
        "ui.menu.play");

    dzemikk::UIActionRegistry::get().registerAction(
        [this, sceneManager, sceneShared](const dzemikk::UIEvent&) {
            if (!std::filesystem::exists("./world.json")) {
                return;
            }

            restartGame();
            sceneManager->setActiveScene(sceneShared);
        },
        "ui.menu.fromcheckpoint");

    dzemikk::UIActionRegistry::get().registerAction(
        [sceneManager, creditsShared](const dzemikk::UIEvent&) {
            sceneManager->setActiveScene(creditsShared);
        },
        "ui.menu.credits");

    dzemikk::UIActionRegistry::get().registerAction(
        [this](const dzemikk::UIEvent&) { _engine->exit(); }, "ui.menu.exit");

    dzemikk::UIActionRegistry::get().registerAction(
        [sceneManager, menuShared](const dzemikk::UIEvent&) {
            sceneManager->setActiveScene(menuShared);
        },
        "ui.credits.back");

    dzemikk::UIActionRegistry::get().registerAction(
        [sceneManager, creditsShared, this](const dzemikk::UIEvent&) {
            setExplorationState();
            _mainScene.get()->findGameObjectByName("Cinematic")->enabled(false);
        },
        "cinematic.endscreen.yes");

    dzemikk::UIActionRegistry::get().registerAction(
        [sceneManager, creditsShared, this](const dzemikk::UIEvent&) { _engine->exit(); },
        "cinematic.endscreen.no");

    _mainScene.get()->findGameObjectByName("UI_RevealPatternBtn")->addComponent<UIPulseEffect>();
    _mainScene.get()->findGameObjectByName("UI_RevealHexBtn")->addComponent<UIPulseEffect>();
    setupInputCallbacks();
}

game::CameraController* Game::getCameraController() {
    return _cameraController;
}

void Game::enableCombatUI(bool enable) {
    auto combatUI = _mainScene.get()->findGameObjectByName("Combat");
    combatUI->enabled(enable);

    auto tooltipsGO = _mainScene.get()->findGameObjectByName("Tooltips_Panel");
    auto patternTooltip = tooltipsGO->findDescendantByName("Pattern");
    patternTooltip->enabled(false);

    auto bonusTooltip = tooltipsGO->findDescendantByName("BonusHex");
    bonusTooltip->enabled(false);

    auto itemTooltip = tooltipsGO->findDescendantByName("Item");
    itemTooltip->enabled(false);
}

dzemikk::AssetHandle<dzemikk::Scene> Game::getCurrentScene() {
    return _mainScene;
}

game::HexGrid* Game::getHexGrid() {
    return _hexGrid;
}

void Game::setupSkybox() {
    auto skyboxShader = _engine->getAssetManager()->get<dzemikk::Shader>("shaders/skybox");

    SkyboxInitContext sCtx(skyboxShader, _engine->getRenderer(), _engine);
    dzemikk::AssetManager::AssetTask<dzemikk::Skybox, SkyboxInitContext> taskSk;
    taskSk.context = sCtx;
    taskSk.onLoad = onSkyboxLoad;
    _engine->getAssetManager()->getAsync("textures/Skybox", taskSk);
}

void Game::setupMainCamera() {
    auto* cameraGO = _mainScene.get()->createGameObject("Camera");

    _mainCamera = cameraGO->addComponent<dzemikk::Camera>();
    auto fxaaProcessEffect = cameraGO->addComponent<dzemikk::OutlinePostProcessEffect>();
    fxaaProcessEffect->setEnabled(false);
    fxaaProcessEffect->setShader(
        _engine->getAssetManager()->get<dzemikk::Shader>("shaders/outline"));
    fxaaProcessEffect->setPriority(1);
    fxaaProcessEffect->setColor(glm::vec3(1.0F, 0.0F, 0.0F));
    _engine->getInput()->OnKeyPressed.addListener(
        [this, fxaaProcessEffect](dzemikk::KeyPressedEvent& event) {
            if (event.GetKeyCode() == GLFW_KEY_F1) {
                _engine->getAssetManager()->reload<dzemikk::Shader>("shaders/outline");
                _engine->getAssetManager()->reload<dzemikk::Shader>("shaders/grain");
            }
            if (event.GetKeyCode() == GLFW_KEY_3) {
                fxaaProcessEffect->setEnabled(false);
            }
            if (event.GetKeyCode() == GLFW_KEY_4) {
                fxaaProcessEffect->setEnabled(false);
            }
        });
    auto postProccessEffect2 = cameraGO->addComponent<dzemikk::PostProcessEffect>();
    postProccessEffect2->setEnabled(false);
    postProccessEffect2->setShader(
        _engine->getAssetManager()->get<dzemikk::Shader>("shaders/grayscale"));
    postProccessEffect2->setPriority(2);

    auto colorGrading = cameraGO->addComponent<dzemikk::ColorGradingEffect>();
    colorGrading->setEnabled(false);
    colorGrading->setShader(
        _engine->getAssetManager()->get<dzemikk::Shader>("shaders/color_grading"));
    colorGrading->setPriority(0);
    colorGrading->setExposure(0.1f);
    colorGrading->setContrast(1.1f);
    colorGrading->setSaturation(1.15f);
    colorGrading->setTemperature(0.1f);
    colorGrading->setTint(-0.05f);

    auto antiAliasing = cameraGO->addComponent<dzemikk::AntiAliasingEffect>();
    antiAliasing->setEnabled(true);
    antiAliasing->setShader(_engine->getAssetManager()->get<dzemikk::Shader>("shaders/fxaa"));
    antiAliasing->setPriority(10);

    _mainCamera = cameraGO->addComponent<dzemikk::Camera>();
    auto postProccessEffect = cameraGO->addComponent<dzemikk::PostProcessEffect>();
    postProccessEffect->setEnabled(false);
    postProccessEffect->setShader(
        _engine->getAssetManager()->get<dzemikk::Shader>("shaders/vignette"));
    postProccessEffect->setPriority(5);

    _engine->getRenderer()->getCameraSystem().setActiveSceneCamera(_mainCamera);
}

void Game::setupWorld() {
    auto shader = _engine->getAssetManager()->get<dzemikk::Shader>("shaders/tile1");
    auto material = std::make_shared<dzemikk::Material>();
    material->setShader(shader);
    material->setAlbedoTexture(_engine->getAssetManager()->get<dzemikk::Texture>(
        "models/assets/hexy/hex_wypukly/hex_wypukly_BaseColor.png"));
    material->setEmissiveTexture(_engine->getAssetManager()->get<dzemikk::Texture>(
        "models/assets/hexy/hex_wypukly/hex_wypukly_Emissive.png"));
    material->setMetallicTexture(_engine->getAssetManager()->get<dzemikk::Texture>(
        "models/assets/hexy/hex_wypukly/hex_wypukly_Metallic.png"));
    material->setRoughnessTexture(_engine->getAssetManager()->get<dzemikk::Texture>(
        "models/assets/hexy/hex_wypukly/hex_wypukly_Roughness.png"));

    _worldGO = _mainScene.get()->findGameObjectByName("World");
    _worldGO->addTag("World");
    auto* world = _worldGO->addComponent<game::World>(1);
    world->setGame(this);

    world->setAssetManager(_engine->getAssetManager());
    world->setMaterial(material);
    world->setPlayer(_playerEntity);

    if (std::filesystem::exists("./world.json")) {
        std::ifstream in("./world.json");
        nlohmann::json worldData = nlohmann::json::parse(in);
        in.close();
        world->load(worldData);
    } else {
        auto chunkMain0 = world->addChunk({.name = "chunkMain0", .steps = 10});

        auto chunkMain1 = world->addChunk({.parentPersistantId = chunkMain0,
                                           .name = "chunkMain1",
                                           .steps = 7,
                                           .dirFromParent = game::HexCoord::Direction::R0});

        auto chunkMain2 = world->addChunk({.parentPersistantId = chunkMain1,
                                           .name = "chunkMain2",
                                           .steps = 12,
                                           .dirFromParent = game::HexCoord::Direction::R0});

        auto chunkMain2Sub1 =
            world->addChunk({.parentPersistantId = chunkMain2,
                             .name = "chunkMain2Sub1",
                             .steps = 15,
                             .dirFromParent = game::HexCoord::Direction::R330,
                             .unlockPattern = game::HexPattern(
                                 {{-1, 1}, {0, 0}, {1, -1}}, game::HexPattern::Type::ATK, 12.0F)});

        auto chunkMain3 = world->addChunk({.parentPersistantId = chunkMain2,
                                           .name = "chunkMain3",
                                           .steps = 17,
                                           .dirFromParent = game::HexCoord::Direction::R30});

        auto chunkMain3Sub1 = world->addChunk({.parentPersistantId = chunkMain3,
                                               .name = "chunkMain3Sub1",
                                               .steps = 10,
                                               .dirFromParent = game::HexCoord::Direction::R90});

        auto chunkMain4 = world->addChunk({.parentPersistantId = chunkMain3,
                                           .name = "chunkMain4",
                                           .steps = 15,
                                           .dirFromParent = game::HexCoord::Direction::R30});

        auto chunkMain5 = world->addChunk({.parentPersistantId = chunkMain4,
                                           .name = "chunkMain5",
                                           .steps = 18,
                                           .dirFromParent = game::HexCoord::Direction::R330});

        auto chunkMain6 = world->addChunk({.parentPersistantId = chunkMain5,
                                           .name = "chunkMain6",
                                           .steps = 15,
                                           .dirFromParent = game::HexCoord::Direction::R30});

        auto chunkMain7 = world->addChunk({.parentPersistantId = chunkMain6,
                                           .name = "chunkMain7",
                                           .steps = 24,
                                           .dirFromParent = game::HexCoord::Direction::R0});

        auto chunkMain4Sub1 = world->addChunk({.parentPersistantId = chunkMain4,
                                               .name = "chunkMain4Sub1",
                                               .steps = 13,
                                               .dirFromParent = game::HexCoord::Direction::R30});

        auto chunkMain7Sub1 = world->addChunk({.parentPersistantId = chunkMain7,
                                               .name = "chunkMain7Sub1",
                                               .steps = 22,
                                               .dirFromParent = game::HexCoord::Direction::R330});

        auto chunkMain7Sub2 = world->addChunk({.parentPersistantId = chunkMain7Sub1,
                                               .name = "chunkMain7Sub2",
                                               .steps = 15,
                                               .dirFromParent = game::HexCoord::Direction::R0});

        auto chunkMain7Sub3 = world->addChunk({.parentPersistantId = chunkMain7Sub2,
                                               .name = "chunkMain7Sub3",
                                               .steps = 17,
                                               .dirFromParent = game::HexCoord::Direction::R330});

        auto chunkMain8 = world->addChunk({.parentPersistantId = chunkMain7,
                                           .name = "chunkMain8",
                                           .steps = 17,
                                           .dirFromParent = game::HexCoord::Direction::R30});

        auto chunkMain9 = world->addChunk({.parentPersistantId = chunkMain8,
                                           .name = "chunkMain9",
                                           .steps = 22,
                                           .dirFromParent = game::HexCoord::Direction::R0});

        auto chunkMain10 = world->addChunk({.parentPersistantId = chunkMain9,
                                            .name = "chunkMain10",
                                            .steps = 30,
                                            .dirFromParent = game::HexCoord::Direction::R0});

        // Removes all hexes with gen state Blocked
        world->getGrid()->clean();
    }
}

void Game::setupWorldVisuals() {
    auto worldVisualManager = _worldGO->addComponent<game::WorldVisualManager>();
    worldVisualManager->setWorld(_worldGO->getComponent<game::World>());
    worldVisualManager->setAssetManager(_engine->getAssetManager());
    worldVisualManager->setGame(this);
    worldVisualManager->init();
    worldVisualManager->generatePathBetweenChunks(
        "chunkMain0", "chunkMain1", game::HexCell::Type::Normal, game::HexCell::Type::Bridge);
    worldVisualManager->generatePathBetweenChunks("chunkMain1", "chunkMain2",
                                                  game::HexCell::Type::Bridge,
                                                  game::HexCell::Type::BlockingBridge);
    worldVisualManager->generatePathBetweenChunks("chunkMain2", "chunkMain3",
                                                  game::HexCell::Type::BlockingBridge,
                                                  game::HexCell::Type::BlockingBridge);
    worldVisualManager->generatePathBetweenChunks("chunkMain3", "chunkMain4",
                                                  game::HexCell::Type::BlockingBridge,
                                                  game::HexCell::Type::BlockingBridge);
    worldVisualManager->generatePathBetweenChunks("chunkMain4", "chunkMain5",
                                                  game::HexCell::Type::BlockingBridge,
                                                  game::HexCell::Type::BlockingBridge);
    worldVisualManager->generatePathBetweenChunks("chunkMain5", "chunkMain6",
                                                  game::HexCell::Type::BlockingBridge,
                                                  game::HexCell::Type::BlockingBridge);
    worldVisualManager->generatePathBetweenChunks("chunkMain6", "chunkMain7",
                                                  game::HexCell::Type::BlockingBridge,
                                                  game::HexCell::Type::BlockingBridge);
    worldVisualManager->generatePathBetweenChunks("chunkMain7", "chunkMain8",
                                                  game::HexCell::Type::BlockingBridge,
                                                  game::HexCell::Type::BlockingBridge);
    worldVisualManager->generatePathBetweenChunks("chunkMain8", "chunkMain9",
                                                  game::HexCell::Type::BlockingBridge,
                                                  game::HexCell::Type::BlockingBridge);
    worldVisualManager->generatePathBetweenChunks("chunkMain9", "chunkMain10",
                                                  game::HexCell::Type::BlockingBridge,
                                                  game::HexCell::Type::BlockingBridge);
    worldVisualManager->generatePathBetweenChunks("chunkMain10", "",
                                                  game::HexCell::Type::BlockingBridge,
                                                  game::HexCell::Type::EnemyBattleHex);

    worldVisualManager->spawnSignToChunk("chunkMain0", "chunkMain1");
    worldVisualManager->spawnSignToChunk("chunkMain1", "chunkMain2");
    worldVisualManager->spawnSignToChunk("chunkMain2", "chunkMain3");
    worldVisualManager->spawnSignToChunk("chunkMain3", "chunkMain4");
    worldVisualManager->spawnSignToChunk("chunkMain4", "chunkMain5");
    worldVisualManager->spawnSignToChunk("chunkMain5", "chunkMain6");
    worldVisualManager->spawnSignToChunk("chunkMain6", "chunkMain7");
    worldVisualManager->spawnSignToChunk("chunkMain7", "chunkMain8");
    worldVisualManager->spawnSignToChunk("chunkMain8", "chunkMain9");
    worldVisualManager->spawnSignToChunk("chunkMain9", "chunkMain10");

    worldVisualManager->spawnForestChunk("chunkMain0");
    worldVisualManager->spawnForestChunk("chunkMain1");
    worldVisualManager->spawnForestChunk("chunkMain2");
    worldVisualManager->spawnForestChunk("chunkMain2Sub1");
    worldVisualManager->spawnForestChunk("chunkMain3");
    worldVisualManager->spawnForestChunk("chunkMain3Sub1");
    worldVisualManager->spawnForestChunk("chunkMain4");
    worldVisualManager->spawnForestChunk("chunkMain5");
    worldVisualManager->spawnForestChunk("chunkMain6");
    worldVisualManager->spawnForestChunk("chunkMain7");
    worldVisualManager->spawnForestChunk("chunkMain4Sub1");
    worldVisualManager->spawnForestChunk("chunkMain7Sub1");
    worldVisualManager->spawnForestChunk("chunkMain7Sub2");
    worldVisualManager->spawnForestChunk("chunkMain7Sub3");
    worldVisualManager->spawnForestChunk("chunkMain8");
    worldVisualManager->spawnForestChunk("chunkMain9");
    worldVisualManager->spawnForestChunk("chunkMain10");
}

void Game::setupUICamera() {
    auto* uiCameraGO = _mainScene.get()->createGameObject("UICamera");
    uiCameraGO->transform()->setPosition({0.0F, 0.0F, 1.0F});
    auto* uiCamera = uiCameraGO->addComponent<dzemikk::Camera>();
    uiCamera->setOrthographic(0.0F, 1920.0F, 0.0F, 1080.0F, -1.0F, 1.0F);
    _engine->getRenderer()->getCameraSystem().setActiveUICamera(uiCamera);
}

void Game::setupInputCallbacks() {
    static dzemikk::MeshRenderer* lastHitRenderer = nullptr;

    static std::unordered_map<dzemikk::MeshRenderer*, std::shared_ptr<dzemikk::Material>>
        baseMaterials;

    _engine->SetUserUpdateCallback([this]() {
        if (!_engine || !_engine->getInput() ||
            !_stateMachine->getCurrentStateAs<game::ExplorationState>()) {
            return;
        }

        int windowWidth = 0;
        int windowHeight = 0;

        glfwGetWindowSize(_engine->getWindow()->nativeHandle(), &windowWidth, &windowHeight);

        dzemikk::Collider* collider = _engine->getCollisions()->raycast(
            _engine->getRenderer()->getCameraSystem().getActiveSceneCamera(), nullptr,
            _engine->getInput()->GetMousePosition(), windowWidth, windowHeight);

        dzemikk::MeshRenderer* currentRenderer = nullptr;

        if (collider) {
            currentRenderer = collider->getOwner()->getComponent<dzemikk::MeshRenderer>();
        }

        constexpr float hoverStrength = 0.5F;

        if (currentRenderer != lastHitRenderer) {

            if (lastHitRenderer && lastHitRenderer->isValid()) {
                auto it = baseMaterials.find(lastHitRenderer);
                if (it != baseMaterials.end()) {
                    lastHitRenderer->setMaterial(0, it->second);
                }
            }

            if (currentRenderer && currentRenderer->isValid()) {

                baseMaterials[currentRenderer] = currentRenderer->getMaterial(0)->clone();

                auto hovered = currentRenderer->getMaterial(0)->clone();
                hovered->setAlbedoColor(baseMaterials[currentRenderer]->getAlbedoColor() *
                                        hoverStrength);

                currentRenderer->setMaterial(0, hovered);
            }

            lastHitRenderer = currentRenderer;
        }
    });

    _engine->getInput()->OnKeyPressed.addListener([this](dzemikk::KeyPressedEvent& event) {
        auto* dialogManagerGo = _mainScene.get()->findGameObjectByTag("DialogManager");
        auto* dialogManager = dialogManagerGo->getComponent<game::DialogManager>();

        if (event.GetKeyCode() == GLFW_KEY_ESCAPE &&
            _stateMachine->getCurrentStateAs<game::ExplorationState>() &&
            !dialogManager->isInDialog()) {
            auto* btnResetInteractable = _menuScene.get()
                                             ->findGameObjectByName("ResetButton")
                                             ->getComponent<dzemikk::IUIInteractable>();
            if (!std::filesystem::exists("./world.json")) {
                btnResetInteractable->setInteractable(false);
            } else {
                btnResetInteractable->setInteractable(true);
            }

            auto* btnPlayText = _menuScene.get()
                                    ->findGameObjectByName("PlayButton")
                                    ->getChildren()
                                    .at(0)
                                    ->getComponent<dzemikk::UITextRenderer>();
            if (!_gameStarted) {
                btnPlayText->text = "PLAY";
            } else {
                btnPlayText->text = "RESUME";
            }

            std::shared_ptr<dzemikk::Scene> menuShared(_menuScene.get(), [](dzemikk::Scene*) {});
            _engine->getSceneManager()->setActiveScene(menuShared);
        }
    });

    _engine->getInput()->OnMouseButtonPressed.addListener(
        [this](dzemikk::MouseButtonPressedEvent& event) {
            auto* dialogManagerGo = _mainScene.get()->findGameObjectByTag("DialogManager");
            auto* dialogManager = dialogManagerGo->getComponent<game::DialogManager>();

            if (event.GetMouseButton() != GLFW_MOUSE_BUTTON_LEFT ||
                !_stateMachine->getCurrentStateAs<game::ExplorationState>() ||
                dialogManager->isInDialog()) {
                return;
            }

            int windowWidth = 0;
            int windowHeight = 0;

            glfwGetWindowSize(_engine->getWindow()->nativeHandle(), &windowWidth, &windowHeight);

            dzemikk::Collider* collider = _engine->getCollisions()->raycast(
                _engine->getRenderer()->getCameraSystem().getActiveSceneCamera(), nullptr,
                _engine->getInput()->GetMousePosition(), static_cast<float>(windowWidth),
                static_cast<float>(windowHeight));

            if (collider && _engine->getInput()->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                auto* currentRenderer = collider->getOwner()->getComponent<dzemikk::MeshRenderer>();

                auto* wh = collider->getOwner()->getComponent<game::WorldHex>();

                if (wh && wh->getHexCell()) {
                    _playerMovement->moveTo(wh->getHexCell());
                }
            }
        });
}

void Game::setupPlayer() {
    auto* playerGO = _mainScene.get()->findGameObjectByName("Player");
    _playerGO = playerGO;
    playerGO->addTag("Player");

    auto* inventory = playerGO->addComponent<game::Inventory>();
    inventory->setGame(this);

    auto skeleton =
        playerGO->getComponent<dzemikk::SkinnedMeshRenderer>()->getModel().get()->getSkeleton();
    auto* animator = playerGO->getComponent<dzemikk::Animator>();

    dzemikk::AnimationClip* idleClip = nullptr;
    dzemikk::AnimationClip* forward30Clip = nullptr;
    dzemikk::AnimationClip* forward90Clip = nullptr;
    dzemikk::AnimationClip* forward150Clip = nullptr;
    dzemikk::AnimationClip* forward210Clip = nullptr;
    dzemikk::AnimationClip* forward270Clip = nullptr;
    dzemikk::AnimationClip* forward330Clip = nullptr;

    float animSpeed = 15.0F;

    idleClip = skeleton->getClip("idle");
    idleClip->setLoop(true);

    forward30Clip = skeleton->getClip("300");
    forward30Clip->setLoop(false);
    forward30Clip->setPlaybackSpeed(animSpeed);

    forward90Clip = skeleton->getClip("0");
    forward90Clip->setLoop(false);
    forward90Clip->setPlaybackSpeed(animSpeed);

    forward150Clip = skeleton->getClip("60");
    forward150Clip->setLoop(false);
    forward150Clip->setPlaybackSpeed(animSpeed);

    forward210Clip = skeleton->getClip("120");
    forward210Clip->setLoop(false);
    forward210Clip->setPlaybackSpeed(animSpeed);

    forward270Clip = skeleton->getClip("180");
    forward270Clip->setLoop(false);
    forward270Clip->setPlaybackSpeed(animSpeed);

    forward330Clip = skeleton->getClip("240");
    forward330Clip->setLoop(false);
    forward330Clip->setPlaybackSpeed(animSpeed);

    animator->getStateMachine()->getState("Idle")->setClip(idleClip);
    animator->getStateMachine()->getState("R30")->setClip(forward30Clip);
    animator->getStateMachine()->getState("R90")->setClip(forward90Clip);
    animator->getStateMachine()->getState("R150")->setClip(forward150Clip);
    animator->getStateMachine()->getState("R210")->setClip(forward210Clip);
    animator->getStateMachine()->getState("R270")->setClip(forward270Clip);
    animator->getStateMachine()->getState("R330")->setClip(forward330Clip);

    playerGO->transform()->rotateAround(-90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    playerGO->transform()->setScale(glm::vec3(0.015f, 0.015f, 0.015f));
    animator->setSkeleton(skeleton.get());

    _playerEntity = playerGO->addComponent<game::PlayerEntity>();
    _playerEntity->setGame(this);
    _playerMovement = playerGO->addComponent<game::PlayerMovement>();
    _playerMovement->setPlayerEntity(_playerEntity);
    _playerMovement->setGame(this);
    _playerMovement->setAnimator(animator);
    _playerMovement->setWorld(_worldGO->getComponent<game::World>());

    animator->play("Idle");

    _hexGrid = _worldGO->getComponent<game::World>()->getGrid();
    _playerMovement->setHexGrid(_hexGrid);

    auto* playerPatternStats = playerGO->addComponent<game::PlayerPatternStatsComponent>();

    auto* patternComponent = playerGO->addComponent<game::PlayerPatternComponent>();
    patternComponent->setEngine(_engine);
    patternComponent->setGrid(_hexGrid);
    patternComponent->setPlayerEntity(_playerEntity);
    patternComponent->setGame(this);

    auto* playerPanel = _mainScene.get()->findGameObjectByName("Player_Panel");
    auto* combatPlayerPanel = playerPanel->addComponent<game::CombatUIPanel>(
        true, game::CombatUIPanel::Mode::AvailablePatterns);
    combatPlayerPanel->setPatternsComponent(patternComponent);
    combatPlayerPanel->setAssetManager(_engine->getAssetManager());
    combatPlayerPanel->setCanvas(playerPanel);
    combatPlayerPanel->setEngine(_engine);

    auto* playerHealthGO = _mainScene.get()
                               ->findGameObjectByName("Player_Avatar_Panel")
                               ->findDescendantByName("Health_Holder");
    playerHealthGO->addTag("PlayerHealthSystem");

    auto* playerHealthSystem = playerHealthGO->addComponent<game::HealthSystem>();
    playerHealthSystem->setOwner(playerHealthGO);
    playerHealthSystem->setHealth(300.0F);
    playerHealthSystem->setMaxHealth(300.0F);
    playerHealthSystem->setTextRenderer(
        playerHealthGO->findChildByName("Text")->getComponent<dzemikk::UITextRenderer>());
    playerHealthSystem->setSlider(
        playerHealthGO->findChildByName("Slider")->getComponent<dzemikk::UISlider>());
    // Restore saved player state
    nlohmann::json worldData;

    if (std::filesystem::exists("./world.json")) {
        std::ifstream in("./world.json");
        worldData = nlohmann::json::parse(in);
        in.close();
    }

    if (!worldData.empty() && worldData.contains("player")) {
        const auto& playerData = worldData["player"];

        if (playerData.contains("position")) {
            game::HexCoord coord = playerData["position"].get<game::HexCoord>();
            auto cell = _hexGrid->getCell(coord);
            if (cell) {
                _playerEntity->teleportTo(cell);
            }
        }

        if (playerData.contains("health") && playerData.contains("maxHealth")) {
            playerHealthSystem->setMaxHealth(playerData["maxHealth"].get<float>());
            playerHealthSystem->setHealth(playerData["health"].get<float>());
        }

        if (playerData.contains("patterns")) {
            patternComponent->clearPatterns();
            for (const auto& pj : playerData["patterns"]) {
                game::HexPattern pat = pj.at("pattern").get<game::HexPattern>();
                int count = pj.at("count").get<int>();
                int maxCount = pj.at("maxCount").get<int>();
                patternComponent->addPattern(pat, count, maxCount);
            }
        }

        if (playerData.contains("inventory")) {
            for (const auto& [typeStr, count] : playerData["inventory"].items()) {
                auto type = static_cast<game::ItemEntity::ItemType>(std::stoi(typeStr));
                inventory->addItem(type, count.get<unsigned int>());
            }
        }
    } else {
        _playerEntity->teleportTo(_hexGrid->getCell({0, 0}));

        patternComponent->addPattern(
            game::HexPattern({{0, 0}}, game::HexPattern::Type::ATK, 100.0F), 5, 5);
        patternComponent->addPattern(
            game::HexPattern({{0, 0}, {1, -1}}, game::HexPattern::Type::ATK, 150.0F), 5, 5);
        patternComponent->addPattern(
            game::HexPattern({{0, 0}, {1, -1}, {1, 0}}, game::HexPattern::Type::ATK, 20.0F), 5, 5);
        patternComponent->addPattern(
            game::HexPattern({{0, 0}, {1, -1}, {1, 0}, {0, 1}}, game::HexPattern::Type::ATK, 25.0F),
            5, 5);

        patternComponent->addPattern(game::HexPattern({{0, 0}}, game::HexPattern::Type::DEF, 10.0F),
                                     5, 5);
        patternComponent->addPattern(
            game::HexPattern({{0, 0}, {1, -1}}, game::HexPattern::Type::DEF, 15.0F), 5, 5);
        patternComponent->addPattern(
            game::HexPattern({{0, 0}, {1, -1}, {2, -2}}, game::HexPattern::Type::DEF, 20.0F), 5, 5);
        patternComponent->addPattern(game::HexPattern({{0, 0}, {1, -1}, {1, 0}, {2, -1}},
                                                      game::HexPattern::Type::DEF, 25.0F),
                                     5, 5);

        patternComponent->addPattern(game::HexPattern({{0, 0}}, game::HexPattern::Type::HEAL, 5.0F),
                                     5, 5);
        patternComponent->addPattern(
            game::HexPattern({{0, 0}, {1, -1}}, game::HexPattern::Type::HEAL, 8.0F), 5, 5);
        patternComponent->addPattern(
            game::HexPattern({{0, 0}, {1, -1}, {0, -1}}, game::HexPattern::Type::HEAL, 1.0F), 5, 5);
        patternComponent->addPattern(game::HexPattern({{0, 0}, {1, -1}, {0, -1}, {-1, 0}},
                                                      game::HexPattern::Type::HEAL, 2.0F),
                                     5, 5);
    }
}

void Game::setupEnemies() {
    if (_worldGO == nullptr || _worldGO->getComponent<game::World>() == nullptr) {
        return;
    }

    auto* enemyManagerGO = _mainScene.get()->findGameObjectByName("EnemyManager");
    enemyManagerGO->addTag("EnemyManager");

    auto* manager = enemyManagerGO->addComponent<game::EnemyManager>();
    manager->setWorld(_worldGO->getComponent<game::World>());
    manager->setAssetManager(_engine->getAssetManager());

    auto* world = _worldGO->getComponent<game::World>();

    if (!world) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[Game] World is not set up. Enemies will not be initialized.");
#endif
        return;
    }

    nlohmann::json worldData;
    if (std::filesystem::exists("./world.json")) {
        std::ifstream in("./world.json");
        worldData = nlohmann::json::parse(in);
        in.close();
    }

    if (!worldData.empty() && worldData.contains("enemies")) {
        manager->loadState(worldData["enemies"]);
    } else {
        game::EnemySpawnConfig chunkMain1Config = {
            .personality = game::EnemyPersonality::Aggressive,
            .type = game::EnemyType::Normal,
            .hp = 100,
            .territoryPattern = "1",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain2")->getPersistantId()},
        };
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain1")->getPersistantId(),
                          chunkMain1Config);

        game::EnemySpawnConfig chunkMain2Config = {
            .personality = game::EnemyPersonality::Balanced,
            .type = game::EnemyType::Normal,
            .hp = 150,
            .territoryPattern = "2",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain3")->getPersistantId()},
        };
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain2")->getPersistantId(),
                          chunkMain2Config);

        game::EnemySpawnConfig chunkMain3Config1 = {
            .personality = game::EnemyPersonality::Aggressive,
            .type = game::EnemyType::Special,
            .hp = 200,
            .territoryPattern = "2",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain3Sub1")->getPersistantId()}};
        game::EnemySpawnConfig chunkMain3Config2 = {
            .personality = game::EnemyPersonality::Defensive,
            .type = game::EnemyType::Normal,
            .hp = 200,
            .territoryPattern = "3",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain4")->getPersistantId()}};
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain3")->getPersistantId(),
                          chunkMain3Config1);
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain3")->getPersistantId(),
                          chunkMain3Config2);

        game::EnemySpawnConfig chunkMain4Config1 = {
            .personality = game::EnemyPersonality::Aggressive,
            .type = game::EnemyType::Boss,
            .hp = 250,
            .territoryPattern = "5",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain5")->getPersistantId()}};
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain4")->getPersistantId(),
                          chunkMain4Config1);

        game::EnemySpawnConfig chunkMain5Config1 = {
            .personality = game::EnemyPersonality::Balanced,
            .type = game::EnemyType::Normal,
            .hp = 300,
            .territoryPattern = "3",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain6")->getPersistantId()}};
        game::EnemySpawnConfig chunkMain5Config2 = {
            .personality = game::EnemyPersonality::Aggressive,
            .type = game::EnemyType::Normal,
            .hp = 250,
            .territoryPattern = "3",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain6")->getPersistantId()}};
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain5")->getPersistantId(),
                          chunkMain5Config1);
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain5")->getPersistantId(),
                          chunkMain5Config2);

        game::EnemySpawnConfig chunkMain6Config = {
            .personality = game::EnemyPersonality::Balanced,
            .type = game::EnemyType::Normal,
            .hp = 350,
            .territoryPattern = "4",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain7")->getPersistantId()}};
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain6")->getPersistantId(),
                          chunkMain6Config);

        game::EnemySpawnConfig chunkMain7Config1 = {
            .personality = game::EnemyPersonality::Defensive,
            .type = game::EnemyType::Normal,
            .hp = 250,
            .territoryPattern = "3",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain8")->getPersistantId()}};
        game::EnemySpawnConfig chunkMain7Config2 = {
            .personality = game::EnemyPersonality::Defensive,
            .type = game::EnemyType::Normal,
            .hp = 300,
            .territoryPattern = "3",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain8")->getPersistantId()}};
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain7")->getPersistantId(),
                          chunkMain7Config1);
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain7")->getPersistantId(),
                          chunkMain7Config2);

        game::EnemySpawnConfig chunkMain7Sub1Config = {
            .personality = game::EnemyPersonality::Defensive,
            .type = game::EnemyType::Special,
            .hp = 400,
            .territoryPattern = "5",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain7Sub2")->getPersistantId()}};
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain7Sub1")->getPersistantId(),
                          chunkMain7Sub1Config);

        game::EnemySpawnConfig chunkMain7Sub2Config = {
            .personality = game::EnemyPersonality::Defensive,
            .type = game::EnemyType::Normal,
            .hp = 350,
            .territoryPattern = "4",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain7Sub3")->getPersistantId()},
        };
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain7Sub2")->getPersistantId(),
                          chunkMain7Sub2Config);

        game::EnemySpawnConfig chunkMain8Config1 = {
            .personality = game::EnemyPersonality::Aggressive,
            .type = game::EnemyType::Normal,
            .hp = 350,
            .territoryPattern = "4",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain9")->getPersistantId()}};
        game::EnemySpawnConfig chunkMain8Config2 = {
            .personality = game::EnemyPersonality::Balanced,
            .count = 1,
            .hp = 400,
            .territoryPattern = "5",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain9")->getPersistantId()}};
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain8")->getPersistantId(),
                          chunkMain8Config1);
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain8")->getPersistantId(),
                          chunkMain8Config2);

        game::EnemySpawnConfig chunkMain9Config1 = {
            .personality = game::EnemyPersonality::Balanced,
            .type = game::EnemyType::Normal,
            .count = 1,
            .hp = 350,
            .territoryPattern = "4",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain10")->getPersistantId()}};
        game::EnemySpawnConfig chunkMain9Config2 = {
            .personality = game::EnemyPersonality::Aggressive,
            .type = game::EnemyType::Normal,
            .count = 1,
            .hp = 300,
            .territoryPattern = "1",
            .blocksChunks = {_hexGrid->getChunkByName("chunkMain10")->getPersistantId()}};
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain9")->getPersistantId(),
                          chunkMain9Config1);
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain9")->getPersistantId(),
                          chunkMain9Config2);

        game::EnemySpawnConfig chunkMain10Config = {
            .personality = game::EnemyPersonality::Aggressive,
            .type = game::EnemyType::Normal,
            .count = 1,
            .hp = 500,
            .territoryPattern = "6",
        };
        manager->addEnemy(_hexGrid->getChunkByName("chunkMain10")->getPersistantId(),
                          chunkMain10Config);
    }

    auto* enemyPatternComponent = enemyManagerGO->addComponent<game::EnemyPatternComponent>();
    auto* enemyPanel = _mainScene.get()->findGameObjectByName("Enemy_Panel");
    auto* combatEnamyPanel =
        enemyPanel->addComponent<game::CombatUIPanel>(false, game::CombatUIPanel::Mode::EnemyUsage);
    combatEnamyPanel->setPatternsComponent(enemyPatternComponent);
    combatEnamyPanel->setAssetManager(_engine->getAssetManager());
    combatEnamyPanel->setCanvas(enemyPanel);
    combatEnamyPanel->setHideEmptyPatterns(true);
    combatEnamyPanel->setEngine(_engine);

    auto* enemyHealthGO = _mainScene.get()
                              ->findGameObjectByName("Enemy_Avatar_Panel")
                              ->findDescendantByName("Health_Holder");

    auto* enemyHealthSystem = enemyHealthGO->addComponent<game::HealthSystem>();
    enemyHealthSystem->setOwner(enemyHealthGO);
    enemyHealthSystem->setTextRenderer(
        enemyHealthGO->findChildByName("Text")->getComponent<dzemikk::UITextRenderer>());
    enemyHealthSystem->setSlider(
        enemyHealthGO->findChildByName("Slider")->getComponent<dzemikk::UISlider>());
}

void Game::setupItems() {
    if (_worldGO == nullptr || _worldGO->getComponent<game::World>() == nullptr) {
        return;
    }

    auto* go = _mainScene.get()->createGameObject("ItemManager");
    go->addTag("ItemManager");
    auto* manager = go->addComponent<game::ItemManager>(1);
    manager->setWorld(_worldGO->getComponent<game::World>());
    manager->setAssetManager(_engine->getAssetManager());
    manager->setGame(this);

    auto* world = _worldGO->getComponent<game::World>();

    if (!world) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[Game] World is not set up. Items will not be initialized.");
#endif
        return;
    }

    nlohmann::json worldData;
    if (std::filesystem::exists("./world.json")) {
        std::ifstream in("./world.json");
        worldData = nlohmann::json::parse(in);
    }

    if (!worldData.empty() && worldData.contains("items")) {
        manager->loadState(worldData["items"]);
    } else {
        // Heal Item setup
        auto healChunks = {"chunkMain2Sub1", "chunkMain3",     "chunkMain3Sub1", "chunkMain4Sub1",
                           "chunkMain7",     "chunkMain7Sub2", "chunkMain9"};
        for (const auto& name : healChunks) {
            manager->addItem(world->getGrid()->getChunkByName(name)->getPersistantId(),
                             {.type = game::ItemEntity::ItemType::Heal, .healAmount = 10.0F});
        }

        // Reveal Pattern Item setup
        auto revealPatternChunks = {
            "chunkMain2Sub1", "chunkMain3Sub1", "chunkMain4",     "chunkMain4Sub1", "chunkMain5",
            "chunkMain6",     "chunkMain7",     "chunkMain7Sub1", "chunkMain7Sub3", "chunkMain9"};
        for (const auto& name : revealPatternChunks) {
            manager->addItem(world->getGrid()->getChunkByName(name)->getPersistantId(),
                             {.type = game::ItemEntity::ItemType::RevealPattern});
        }

        // Reveal Hex Item setup
        auto revealHexChunks = {"chunkMain2Sub1", "chunkMain4",     "chunkMain3Sub1",
                                "chunkMain6",     "chunkMain7Sub1", "chunkMain7Sub2"};
        for (const auto& name : revealHexChunks) {
            manager->addItem(world->getGrid()->getChunkByName(name)->getPersistantId(),
                             {.type = game::ItemEntity::ItemType::RevealHex});
        }

        // Bonus Hex Item Setup
        game::HexPattern pat = game::HexPattern({{0, 0}}, game::HexPattern::Type::BONUSHEX);
        auto bonusHex = {"chunkMain2Sub1", "chunkMain4Sub1", "chunkMain3Sub1", "chunkMain7",
                         "chunkMain7Sub1", "chunkMain7Sub3", "chunkMain7Sub3"};
        for (const auto& name : bonusHex) {
            manager->addItem(world->getGrid()->getChunkByName(name)->getPersistantId(),
                             {.type = game::ItemEntity::ItemType::BonusHex, .bonusPattern = pat});
        }
    }
}

void Game::registerDefaultTerritories() {
    game::TerritoryPatternRegistry::instance().registerPattern({.name = "1",
                                                                .offsets = {{1, 0},
                                                                            {1, -1},
                                                                            {0, -1},

                                                                            {-1, 0},
                                                                            {-1, 1},
                                                                            {0, 1}}});

    game::TerritoryPatternRegistry::instance().registerPattern({.name = "2",
                                                                .offsets = {{1, 0},
                                                                            {1, -1},
                                                                            {0, -1},

                                                                            {-1, 0},
                                                                            {-1, 1},
                                                                            {0, 1},

                                                                            {1, 1},
                                                                            {-1, -1}}});

    game::TerritoryPatternRegistry::instance().registerPattern({.name = "3",
                                                                .offsets = {
                                                                    {1, 0},
                                                                    {1, -1},
                                                                    {0, -1},
                                                                    {-1, 0},
                                                                    {-1, 1},
                                                                    {0, 1},

                                                                    {2, 0},
                                                                    {0, -2},
                                                                    {-2, 0},
                                                                    {0, 2},
                                                                    {2, -2},
                                                                    {-2, 2},
                                                                }});

    game::TerritoryPatternRegistry::instance().registerPattern({.name = "4",
                                                                .offsets = {
                                                                    {1, 0},
                                                                    {1, -1},
                                                                    {0, -1},
                                                                    {-1, 0},
                                                                    {-1, 1},
                                                                    {0, 1},

                                                                    {2, 0},
                                                                    {2, -1},
                                                                    {1, -2},
                                                                    {-1, -1},
                                                                    {-2, 0},
                                                                    {-2, 1},
                                                                    {-1, 2},
                                                                    {1, 1},
                                                                    {2, -2},
                                                                    {-2, 2},
                                                                }});

    game::TerritoryPatternRegistry::instance().registerPattern({.name = "5",
                                                                .offsets = {
                                                                    {1, 0},
                                                                    {1, -1},
                                                                    {0, -1},
                                                                    {-1, 0},
                                                                    {-1, 1},
                                                                    {0, 1},

                                                                    {2, 0},
                                                                    {2, -1},
                                                                    {1, -2},
                                                                    {0, -2},
                                                                    {-1, -1},
                                                                    {-2, 0},
                                                                    {-2, 1},
                                                                    {-1, 2},
                                                                    {0, 2},
                                                                    {1, 1},
                                                                    {2, -2},
                                                                    {-2, 2},
                                                                }});

    game::TerritoryPatternRegistry::instance().registerPattern(
        {.name = "6",
         .offsets = {
             {1, 0},  {1, -1}, {0, -1}, {-1, 0}, {-1, 1},  {0, 1},

             {2, 0},  {2, -1}, {1, -2}, {0, -2}, {-1, -1}, {-2, 0},
             {-2, 1}, {-1, 2}, {0, 2},  {1, 1},  {2, -2},  {-2, 2},

             {3, 0},  {3, -1}, {2, -3}, {1, -3}, {-2, -1}, {-3, 0},
             {-3, 1}, {-2, 3}, {-1, 3}, {2, 1},
         }});
}

void Game::setExplorationState() {
    _stateMachine->setState(std::make_unique<game::ExplorationState>(this));
}

void Game::setCinematicState() {
    _stateMachine->setState(std::make_unique<game::CinematicState>(this));
}

void Game::setupTotems() {
    if (_worldGO == nullptr || _worldGO->getComponent<game::World>() == nullptr) {
        return;
    }

    auto* go = _mainScene.get()->createGameObject("TotemManager");
    go->addTag("TotemManager");

    auto* manager = go->addComponent<game::TotemManager>(2);
    manager->setWorld(_worldGO->getComponent<game::World>());
    manager->setAssetManager(_engine->getAssetManager());
    manager->setGame(this);

    auto* world = _worldGO->getComponent<game::World>();

    if (!world) {
        return;
    }

    nlohmann::json worldData;
    if (std::filesystem::exists("./world.json")) {
        std::ifstream in("./world.json");
        worldData = nlohmann::json::parse(in);
        in.close();
    }

    if (!worldData.empty() && worldData.contains("totems")) {
        manager->loadState(worldData["totems"]);
    } else {
        manager->addTotem(_hexGrid->getChunkByName("chunkMain2")->getPersistantId(),
                          {.persistantId = boost::uuids::string_generator()(
                               "478657ac-c332-43a8-b9bb-3aca14c32662"),
                           .pattern = game::HexPattern({{-1, 1}, {0, 0}, {1, -1}},
                                                       game::HexPattern::Type::ATK, 12.0F)});
    }
}

void Game::setupDialogs() {
    if (_worldGO == nullptr || _worldGO->getComponent<game::World>() == nullptr) {
        return;
    }

    auto* go = _mainScene.get()->createGameObject("DialogManager");
    go->addTag("DialogManager");

    auto* manager = go->addComponent<game::DialogManager>();
    manager->setWorld(_worldGO->getComponent<game::World>());
    manager->setAssetManager(_engine->getAssetManager());
    manager->setGame(this);

    auto* world = _worldGO->getComponent<game::World>();

    if (!world) {
        return;
    }

    auto prefab =
        _engine->getAssetManager()->get<nlohmann::json>("prefabs/totem/totem_dialog.prefab");
    auto totem = dzemikk::PrefabSerializer::instantiate(*_mainScene.get(), *prefab.get(),
                                                        _engine->getAssetManager());

    auto* rendererTotemGO =
        totem->findChildByName("platform")->getComponent<dzemikk::MeshRenderer>();
    rendererTotemGO->setCullingRadius(60.0F);
    rendererTotemGO = totem->findDescendantByName("Left")->getComponent<dzemikk::MeshRenderer>();
    rendererTotemGO->setCullingRadius(60.0F);
    rendererTotemGO = totem->findDescendantByName("Right")->getComponent<dzemikk::MeshRenderer>();
    rendererTotemGO->setCullingRadius(60.0F);

    rendererTotemGO = totem->findDescendantByName("Totem3")->getComponent<dzemikk::MeshRenderer>();
    rendererTotemGO->setCullingRadius(60.0F);
    rendererTotemGO = totem->findDescendantByName("Totem5")->getComponent<dzemikk::MeshRenderer>();
    rendererTotemGO->setCullingRadius(60.0F);
    rendererTotemGO = totem->findDescendantByName("Totem6")->getComponent<dzemikk::MeshRenderer>();
    rendererTotemGO->setCullingRadius(60.0F);

    auto entity = totem->addComponent<game::TotemDialogEntity>();
    entity->onEnter(world->getGrid()->getCell({5, 0}));

    nlohmann::json worldData;
    if (std::filesystem::exists("./world.json")) {
        std::ifstream in("./world.json");
        worldData = nlohmann::json::parse(in);
        in.close();
    }

    if (!worldData.empty() && worldData.contains("dialogs")) {
        manager->loadState(worldData["dialogs"]);
    } else {
        manager->addDialog({
            .targetEntityId = totem->getComponent<game::TotemDialogEntity>()->getId(),
            .entries =
                {{.speaker = "Mother", .text = "Where am I? Where is my son?!"},
                 {.speaker = "Totem",
                  .text = "You stand upon a land of runes, spirits, and blood offered\n"
                          "to the gods. The shamans have taken your son. They will\n"
                          "sacrifice him in the heart of the volcano."},
                 {.speaker = "Mother", .text = "I would rather die than let them hurt my child!"},
                 {.speaker = "Totem",
                  .text = "The shamans rule these islands, and their servants will \nstand in your "
                          "way. "
                          "Every step will bring you closer \nto your son... or closer to death."},
                 {.speaker = "Mother", .text = "I am not afraid. I will fight them!"},
                 {.speaker = "Totem",
                  .text =
                      "Take these runes. Their power will allow you to \nattack, shield yourself "
                      "from harm, and heal your wounds. \nLearn to wield them, and you may reach "
                      "your son \nbefore the volcano's flames consume him."}},
        });
    }
}

void Game::restartGame() {
    if (!std::filesystem::exists("./world.json")) {
        spdlog::warn("[Game] No world.json file found. Cannot restart.");
        throw std::runtime_error("No world.json file found. Cannot restart.");
    }

    auto in = std::ifstream("./world.json");
    auto checkpoint = nlohmann::json::parse(in);
    in.close();
    auto* scene = _mainScene.get();
    auto* world = _worldGO->getComponent<game::World>();

    auto* enemyManagerGO = scene->findGameObjectByTag("EnemyManager");
    auto* enemyManager = enemyManagerGO->getComponent<game::EnemyManager>();

    auto* itemManagerGO = scene->findGameObjectByTag("ItemManager");
    auto* itemManager = itemManagerGO->getComponent<game::ItemManager>();

    auto* totemManagerGO = scene->findGameObjectByTag("TotemManager");
    auto* totemManager = totemManagerGO->getComponent<game::TotemManager>();

    auto* dialogManagerGO = scene->findGameObjectByTag("DialogManager");
    auto* dialogManager = dialogManagerGO->getComponent<game::DialogManager>();

    world->loadDiff(checkpoint);

    auto currentState = world->save();

    if (checkpoint.contains("player")) {
        const auto& playerData = checkpoint["player"];

        auto* playerHealthGO = scene->findGameObjectByName("Player_Avatar_Panel")
                                   ->findDescendantByName("Health_Holder");
        auto* playerHealthSystem = playerHealthGO->getComponent<game::HealthSystem>();
        if (playerData.contains("health") && playerData.contains("maxHealth")) {
            playerHealthSystem->setMaxHealth(playerData["maxHealth"].get<float>());
            playerHealthSystem->setHealth(playerData["health"].get<float>());
        }

        auto* patternComponent = _playerGO->getComponent<game::PlayerPatternComponent>();
        if (playerData.contains("patterns")) {
            patternComponent->clearPatterns();
            for (const auto& pj : playerData["patterns"]) {
                game::HexPattern pat = pj.at("pattern").get<game::HexPattern>();
                int count = pj.at("count").get<int>();
                int maxCount = pj.at("maxCount").get<int>();
                patternComponent->addPattern(pat, count, maxCount);
            }
            auto* playerPanel = _mainScene.get()->findGameObjectByName("Player_Panel");
            if (auto* combatPlayerPanel = playerPanel->getComponent<game::CombatUIPanel>();
                combatPlayerPanel) {
                combatPlayerPanel->refresh();
            }
        }

        auto* inventory = _playerGO->getComponent<game::Inventory>();
        inventory->clear();
        if (playerData.contains("inventory")) {
            for (const auto& [typeStr, count] : playerData["inventory"].items()) {
                auto type = static_cast<game::ItemEntity::ItemType>(std::stoi(typeStr));
                inventory->addItem(type, count.get<unsigned int>());
            }
        }

        if (playerData.contains("position")) {
            auto coord = playerData["position"].get<game::HexCoord>();
            auto cell = _hexGrid->getCell(coord);
            if (cell) {
                _playerEntity->teleportTo(cell);
            }
        }
    }

    std::unordered_set<std::string> currentItemIds;
    if (currentState.contains("items")) {
        for (const auto& [idStr, _] : currentState["items"].items()) {
            currentItemIds.insert(idStr);
        }
    }

    if (checkpoint.contains("items")) {
        for (const auto& [idStr, itemData] : checkpoint["items"].items()) {
            if (!currentItemIds.contains(idStr)) {
                auto cfg = itemData.get<game::ItemSpawnConfig>();
                game::HexCoord coord = itemData["gridPos"].get<game::HexCoord>();
                itemManager->addItem(cfg.chunkId, cfg, coord);
            }
        }
    }

    std::unordered_set<std::string> currentEnemyIds;
    if (currentState.contains("enemies")) {
        for (const auto& [idStr, _] : currentState["enemies"].items()) {
            currentEnemyIds.insert(idStr);
        }
    }

    if (checkpoint.contains("enemies")) {
        for (const auto& [idStr, enemyData] : checkpoint["enemies"].items()) {
            if (!currentEnemyIds.contains(idStr)) {
                auto cfg = enemyData.get<game::EnemySpawnConfig>();
                game::HexCoord coord = enemyData["gridPos"].get<game::HexCoord>();
                auto chunkId =
                    boost::uuids::string_generator()(enemyData.at("chunkId").get<std::string>());
                cfg.chunkId = chunkId;
                enemyManager->addEnemy(chunkId, cfg, coord);
            }
        }
    }

    std::unordered_set<std::string> currentTotemIds;
    if (currentState.contains("totems")) {
        for (const auto& [idStr, _] : currentState["totems"].items()) {
            currentTotemIds.insert(idStr);
        }
    }

    if (checkpoint.contains("totems")) {
        for (const auto& [idStr, totemData] : checkpoint["totems"].items()) {
            if (!currentTotemIds.contains(idStr)) {
                auto cfg = totemData.get<game::TotemSpawnConfig>();
                game::HexCoord coord = totemData["gridPos"].get<game::HexCoord>();
                totemManager->addTotem(cfg.chunkId, cfg, coord);
            } else if (totemData.contains("used") && totemData["used"].get<bool>()) {
                auto totemId = boost::uuids::string_generator()(idStr);
                totemManager->markTotemUsed(totemId);
            } else if (totemData.contains("used") && !totemData["used"].get<bool>() &&
                       currentTotemIds.contains(idStr) &&
                       currentState["totems"][idStr].contains("used") &&
                       currentState["totems"][idStr]["used"].get<bool>()) {
                totemManager->markTotemUnused(boost::uuids::string_generator()(idStr));
            }
        }
    }

    std::unordered_set<std::string> currentDialogIds;
    if (currentState.contains("dialogs")) {
        for (const auto& [idStr, _] : currentState["dialogs"].items()) {
            currentDialogIds.insert(idStr);
        }
    }

    if (checkpoint.contains("dialogs")) {
        for (const auto& [idStr, dialogData] : checkpoint["dialogs"].items()) {
            if (!currentDialogIds.contains(idStr)) {
                if (dialogData.contains("config")) {
                    auto cfg = dialogData["config"].get<game::DialogSpawnConfig>();
                    dialogManager->addDialog(cfg);
                }
            } else if (dialogData.contains("triggered") && dialogData["triggered"].get<bool>()) {
                auto dialogId = boost::uuids::string_generator()(idStr);
                dialogManager->markDialogTriggered(dialogId);
            } else if (dialogData.contains("triggered") && !dialogData["triggered"].get<bool>() &&
                       currentDialogIds.contains(idStr) &&
                       currentState["dialogs"][idStr].contains("triggered") &&
                       currentState["dialogs"][idStr]["triggered"].get<bool>()) {
                dialogManager->markDialogUntriggered(boost::uuids::string_generator()(idStr));
            }
        }
    }

    _stateMachine->setState(std::make_unique<game::ExplorationState>(this));
}
