#pragma once

#include "game.h"

#include "assetManager/assetmanager.h"
#include "camera/cameraController.h"
#include "collisions/collisions.h"
#include "core/engine.h"
#include "core/time.h"
#include "core/window.h"
#include "ecs/components/camera.h"
#include "ecs/components/collider.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/ui/uiActionRegistry.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/scenemanager.h"
#include "input/input.h"
#include "map/HexCoord.h"
#include "map/PlayerEntity.h"
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
#include <memory>
#include <utility>

#if DZEMIKK_DEV_TOOLS
#include <imgui.h>
#endif

#include "animation/animationclip.h"
#include "ecs/components/colorGradingEffect.h"
#include "ecs/components/fxaaPostProcessEffect.h"
#include "ecs/components/outlinePostProcessEffect.h"
#include "ecs/components/postProcessEffect.h"
#include "enemySystem/enemyManager.h"
#include "enemySystem/enemyPatternComponent.h"
#include "enemySystem/territoryPatternRegistry.h"
#include "player/playerPatternComponent.h"
#include "player/playerPatternStatsComponent.h"
#include "stateMachine/combatState.h"
#include "stateMachine/explorationState.h"
#include "ui/combatUIPanel.h"

#include <animation/animationstatemachine.h>
#include <ecs/components/animator.h>
#include <ecs/components/skinnedMeshRenderer.h>
#include <gameStateMachine.h>
#include <healthSystem.h>
#include <iostream>

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
    game::Perlin perlin(1);

    auto* assetManager = _engine->getAssetManager();
    auto* sceneManager = _engine->getSceneManager();

    setupSkybox();
    _mainScene = assetManager->get<dzemikk::Scene>("scenes/gameplay4-sm.json");


    std::shared_ptr<dzemikk::Scene> sceneShared(_mainScene.get(), [](dzemikk::Scene*) {});
    sceneManager->loadScene(sceneShared);
    sceneManager->setActiveScene(sceneShared);

    setupMainCamera();
    setupUICamera();
    setupWorld();
    setupPlayer();
    registerDefaultTerritories();
    setupEnemies();

    auto root = _mainScene.get()->findGameObjectByName("Root");
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

    setupInputCallbacks();

    _engine->start();
}

game::CameraController* Game::getCameraController() {
    return _cameraController;
}

void Game::enableCombatUI(bool enable) {
    auto combatUI = _mainScene.get()->findGameObjectByName("Combat");
    combatUI->enabled(enable);
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
    fxaaProcessEffect->setShader(_engine->getAssetManager()->get<dzemikk::Shader>("shaders/outline"));
    fxaaProcessEffect->setPriority(1);
    fxaaProcessEffect->setColor(glm::vec3(1.0F, 0.0F, 0.0F));
    _engine->getInput()->OnKeyPressed.addListener
     ([this, fxaaProcessEffect](dzemikk::KeyPressedEvent& event) {
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
    postProccessEffect2->setShader(_engine->getAssetManager()->get<dzemikk::Shader>("shaders/grayscale"));
    postProccessEffect2->setPriority(2);

    auto colorGrading = cameraGO->addComponent<dzemikk::ColorGradingEffect>();
    colorGrading->setEnabled(false);
    colorGrading->setShader(_engine->getAssetManager()->get<dzemikk::Shader>("shaders/color_grading"));
    colorGrading->setPriority(0);
    colorGrading->setExposure(0.1f);
    colorGrading->setContrast(1.1f);
    colorGrading->setSaturation(1.15f);
    colorGrading->setTemperature(0.1f);
    colorGrading->setTint(-0.05f);

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
    auto shader2 = _engine->getAssetManager()->get<dzemikk::Shader>("shaders/tile2");
    auto material = std::make_shared<dzemikk::Material>();
    material->setShader(shader);
    auto material2 = std::make_shared<dzemikk::Material>();
    material2->setShader(shader2);

    auto model = _engine->getAssetManager()->get<dzemikk::Model>("models/hex_wypukly.fbx");

    dzemikk::AssetHandle<dzemikk::Model> resourceModel =
        _engine->getAssetManager()->getPrimitiveModel(
            dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Sphere);

    _worldGO = _mainScene.get()->findGameObjectByName("World");
    auto* world = _worldGO->addComponent<game::World>(1);
    world->setModel(model);
    world->setMaterial(material);
    world->setMaterial2(material2);
    // world->setEnemyModel(enemyModel);
    world->setResourceModel(resourceModel);
    world->setPlayer(_playerEntity);
    world->registerGenerator("full", [](int step, int maxSteps) { return 1.0F; });

    auto c1 = world->addChunk({.steps = 7});
    _chunkIds["c1"] = c1;

    auto c2 = world->addChunk({.parentChunkId = c1,
                               .steps = 12,
                               .dirFromParent = game::HexCoord::Direction::R0}); // connect chunk
    _chunkIds["c2"] = c2;

    auto c3 = world->addChunk(
        {.parentChunkId = c2, .steps = 15, .dirFromParent = game::HexCoord::Direction::R330});
    _chunkIds["c3"] = c3;

    auto c4 = world->addChunk(
        {.parentChunkId = c3, .steps = 11, .dirFromParent = game::HexCoord::Direction::R330});
    _chunkIds["c4"] = c4;

    auto c5 = world->addChunk(
        {.parentChunkId = c4, .steps = 9, .dirFromParent = game::HexCoord::Direction::R0});
    _chunkIds["c5"] = c5;

    auto c6 = world->addChunk(
        {.parentChunkId = c2, .steps = 17, .dirFromParent = game::HexCoord::Direction::R30});
    _chunkIds["c6"] = c6;

    auto c7 = world->addChunk({.parentChunkId = c6,
                               .steps = 15,
                               .dirFromParent = game::HexCoord::Direction::R30}); // connect chunk
    _chunkIds["c7"] = c7;

    auto c8 = world->addChunk(
        {.parentChunkId = c7, .steps = 19, .dirFromParent = game::HexCoord::Direction::R330});
    _chunkIds["c8"] = c8;

    auto c9 = world->addChunk(
        {.parentChunkId = c8, .steps = 15, .dirFromParent = game::HexCoord::Direction::R30});
    _chunkIds["c9"] = c9;

    auto c10 = world->addChunk({.parentChunkId = c9,
                                .steps = 24,
                                .dirFromParent = game::HexCoord::Direction::R0}); // connect chunk
    _chunkIds["c10"] = c10;

    auto c11 = world->addChunk(
        {.parentChunkId = c7, .steps = 14, .dirFromParent = game::HexCoord::Direction::R30});
    _chunkIds["c11"] = c11;

    auto c12 = world->addChunk(
        {.parentChunkId = c10, .steps = 22, .dirFromParent = game::HexCoord::Direction::R330});
    _chunkIds["c12"] = c12;

    auto c13 = world->addChunk(
        {.parentChunkId = c12, .steps = 15, .dirFromParent = game::HexCoord::Direction::R0});
    _chunkIds["c13"] = c13;

    auto c14 = world->addChunk(
        {.parentChunkId = c13, .steps = 17, .dirFromParent = game::HexCoord::Direction::R330});
    _chunkIds["c14"] = c14;

    auto c15 = world->addChunk(
        {.parentChunkId = c10, .steps = 17, .dirFromParent = game::HexCoord::Direction::R30});
    _chunkIds["c15"] = c15;

    auto c16 = world->addChunk(
        {.parentChunkId = c15, .steps = 22, .dirFromParent = game::HexCoord::Direction::R0});
    _chunkIds["c16"] = c16;

    auto c17 = world->addChunk(
        {.parentChunkId = c16, .steps = 30, .dirFromParent = game::HexCoord::Direction::R0});
    _chunkIds["c17"] = c17;

    // Removes all hexes with gen state Blocked
    world->getGrid()->clean();

    std::ofstream out("./world.json");
    out << world->save().dump(4);
    out.close();

    // std::ifstream in("./world.json");
    // nlohmann::json worldData = nlohmann::json::parse(in);
    // world->load(worldData);
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
    static std::unordered_map<dzemikk::MeshRenderer*, glm::vec4> baseColors;

    _engine->SetUserUpdateCallback([this]() {
        auto ensureBase = [&](dzemikk::MeshRenderer* r) { baseColors[r] = r->getColor(); };

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

        constexpr float hoverStrength = 0.5f;

        if (currentRenderer != lastHitRenderer) {

            if (lastHitRenderer && lastHitRenderer->isValid()) {
                auto base = baseColors[lastHitRenderer];
                lastHitRenderer->setColor(base);
            }

            if (currentRenderer && currentRenderer->isValid()) {
                ensureBase(currentRenderer);

                auto base = baseColors[currentRenderer];
                currentRenderer->setColor(base * 0.5f);
            }

            lastHitRenderer = currentRenderer;
        }
    });

    _engine->getInput()->OnMouseButtonPressed.addListener(
        [this](dzemikk::MouseButtonPressedEvent& event) {
            if (event.GetMouseButton() != GLFW_MOUSE_BUTTON_LEFT ||
                !_stateMachine->getCurrentStateAs<game::ExplorationState>()) {
                return;
            }

            int windowWidth = 0;
            int windowHeight = 0;
            glfwGetWindowSize(_engine->getWindow()->nativeHandle(), &windowWidth, &windowHeight);

            dzemikk::Collider* collider = _engine->getCollisions()->raycast(
                _engine->getRenderer()->getCameraSystem().getActiveSceneCamera(), nullptr,
                _engine->getInput()->GetMousePosition(), static_cast<float>(windowWidth),
                static_cast<float>(windowHeight));

            dzemikk::MeshRenderer* currentRenderer = nullptr;

            if (collider) {
                if (_engine->getInput()->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                    currentRenderer = collider->getOwner()->getComponent<dzemikk::MeshRenderer>();
                    auto* wh = collider->getOwner()->getComponent<game::WorldHex>();
                    if (wh != nullptr && wh->getHexCell() != nullptr) {
                        _playerMovement->moveTo(wh->getHexCell());
                    }
                }
            }
        });
}

void Game::setupPlayer() {
    auto playerGO = _mainScene.get()->findGameObjectByName("Player");

    dzemikk::AnimationClip* idleClip = nullptr;
    dzemikk::AnimationClip* forwardClip = nullptr;

    auto skeleton = playerGO->getComponent<dzemikk::SkinnedMeshRenderer>()->getModel().get()->getSkeleton();

    idleClip = skeleton->getClip("idle");
    idleClip->setLoop(true);

    forwardClip = skeleton->getClip("forward_1_0");

    playerGO->transform()->rotateAround(-90.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    auto animator = playerGO->getComponent<dzemikk::Animator>();
    animator->getStateMachine()->getState("Idle")->setClip(idleClip);
    animator->getStateMachine()->getState("R30")->setClip(forwardClip);
    animator->getStateMachine()->getState("R90")->setClip(forwardClip);
    animator->getStateMachine()->getState("R150")->setClip(forwardClip);
    animator->getStateMachine()->getState("R210")->setClip(forwardClip);
    animator->getStateMachine()->getState("R270")->setClip(forwardClip);
    animator->getStateMachine()->getState("R330")->setClip(forwardClip);

    animator->setApplyRootMotion(true);
    animator->setSkeleton(skeleton.get());
    animator->setRootMotionMode(dzemikk::RootMotionMode::Position);

    _playerEntity = playerGO->addComponent<game::PlayerEntity>();
    _playerMovement = playerGO->addComponent<game::PlayerMovement>();
    _playerMovement->setPlayerEntity(_playerEntity);
    _playerMovement->setSpeed(0.25F);
    _playerMovement->setGame(this);
    _playerMovement->setAnimator(animator);

    animator->play("Idle");

    _hexGrid = _worldGO->getComponent<game::World>()->getGrid();
    _playerMovement->setHexGrid(_hexGrid);

    auto playerPatternStats = playerGO->addComponent<game::PlayerPatternStatsComponent>();

    auto patternComponent = playerGO->addComponent<game::PlayerPatternComponent>();
    patternComponent->setEngine(_engine);
    patternComponent->setGrid(_hexGrid);

    auto playerPanel = _mainScene.get()->findGameObjectByName("Player_Panel");
    auto combatPlayerPanel = playerPanel->addComponent<game::CombatUIPanel>(true, game::CombatUIPanel::Mode::AvailablePatterns);
    combatPlayerPanel->setPatternsComponent(patternComponent);
    combatPlayerPanel->setAssetManager(_engine->getAssetManager());
    combatPlayerPanel->setCanvas(playerPanel);

    auto playerHealthbarGO = _mainScene.get()->findGameObjectByName("Player_Healthbar_Slider");

    auto playerHealthSystem = playerHealthbarGO->addComponent<game::HealthSystem>();
    playerHealthSystem->setSlider(playerHealthbarGO->getComponent<dzemikk::UISlider>());
    playerHealthSystem->setMaxHealth(30.0F);
    playerHealthSystem->setHealth(30.0F);
}

void Game::setupEnemies() {
    auto* enemyManagerGO = _mainScene.get()->findGameObjectByName("EnemyManager");

    auto* enemyManager = enemyManagerGO->addComponent<game::EnemyManager>();

    enemyManager->setWorld(_worldGO->getComponent<game::World>());
    enemyManager->setAssetManager(_engine->getAssetManager());

    std::vector<game::EnemyManager::EnemySpawnConfig> c1Config = {
        {game::EnemyPersonality::Aggressive, game::EnemyType::Normal, 1, 15, "1"}};
    enemyManager->setSpawnConfig(_chunkIds["c1"], c1Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c2Config = {
        {game::EnemyPersonality::Balanced, game::EnemyType::Normal, 1, 20, "2"}};
    enemyManager->setSpawnConfig(_chunkIds["c2"], c2Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c3Config = {
        {game::EnemyPersonality::Aggressive, game::EnemyType::Special, 1, 30, "3"},
    };
    enemyManager->setSpawnConfig(_chunkIds["c3"], c3Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c4Config = {
        {game::EnemyPersonality::Aggressive, game::EnemyType::Normal, 1, 20, "2"},
    };
    enemyManager->setSpawnConfig(_chunkIds["c4"], c4Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c6Config = {
        {game::EnemyPersonality::Aggressive, game::EnemyType::Normal, 1, 20, "2"},
        {game::EnemyPersonality::Defensive, game::EnemyType::Normal, 1, 25, "3"},
    };
    enemyManager->setSpawnConfig(_chunkIds["c6"], c6Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c7Config = {
        {game::EnemyPersonality::Defensive, game::EnemyType::Normal, 1, 25, "3"},
        {game::EnemyPersonality::Balanced, game::EnemyType::Special, 1, 35, "4"},
    };
    enemyManager->setSpawnConfig(_chunkIds["c7"], c7Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c8Config = {
        {game::EnemyPersonality::Balanced, game::EnemyType::Normal, 1, 30, "3"},
        {game::EnemyPersonality::Aggressive, game::EnemyType::Normal, 1, 25, "3"},
    };
    enemyManager->setSpawnConfig(_chunkIds["c8"], c8Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c9Config = {
        {game::EnemyPersonality::Balanced, game::EnemyType::Normal, 1, 35, "4"},
    };
    enemyManager->setSpawnConfig(_chunkIds["c9"], c9Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c10Config = {
        {game::EnemyPersonality::Defensive, game::EnemyType::Normal, 1, 25, "3"},
        {game::EnemyPersonality::Defensive, game::EnemyType::Normal, 1, 30, "3"},
    };
    enemyManager->setSpawnConfig(_chunkIds["c10"], c10Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c12Config = {
        {game::EnemyPersonality::Defensive, game::EnemyType::Special, 1, 40, "5"},
    };
    enemyManager->setSpawnConfig(_chunkIds["c12"], c12Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c13Config = {
        {game::EnemyPersonality::Defensive, game::EnemyType::Normal, 1, 35, "4"},
    };
    enemyManager->setSpawnConfig(_chunkIds["c13"], c13Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c15Config = {
        {game::EnemyPersonality::Aggressive, game::EnemyType::Normal, 1, 35, "4"},
        {game::EnemyPersonality::Balanced, game::EnemyType::Normal, 1, 40, "5"},
    };
    enemyManager->setSpawnConfig(_chunkIds["c15"], c15Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c16Config = {
        {game::EnemyPersonality::Balanced, game::EnemyType::Normal, 1, 35, "4"},
        {game::EnemyPersonality::Aggressive, game::EnemyType::Normal, 1, 30, "1"},
    };
    enemyManager->setSpawnConfig(_chunkIds["c16"], c16Config);

    std::vector<game::EnemyManager::EnemySpawnConfig> c17Config = {
        {game::EnemyPersonality::Aggressive, game::EnemyType::Normal, 1, 50, "6"},
    };
    enemyManager->setSpawnConfig(_chunkIds["c17"], c17Config);

    enemyManager->spawnEnemiesPerChunk();

    auto enemyPatternComponent = enemyManagerGO->addComponent<game::EnemyPatternComponent>();
    auto enemyPanel = _mainScene.get()->findGameObjectByName("Enemy_Panel");
    auto combatEnamyPanel =
        enemyPanel->addComponent<game::CombatUIPanel>(false, game::CombatUIPanel::Mode::EnemyUsage);
    combatEnamyPanel->setPatternsComponent(enemyPatternComponent);
    combatEnamyPanel->setAssetManager(_engine->getAssetManager());
    combatEnamyPanel->setCanvas(enemyPanel);

    auto enemyHealthbarGO = _mainScene.get()->findGameObjectByName("Enemy_Healthbar_Slider");

    auto enemyHealthSystem = enemyHealthbarGO->addComponent<game::HealthSystem>();
    enemyHealthSystem->setSlider(enemyHealthbarGO->getComponent<dzemikk::UISlider>());
}

void Game::registerDefaultTerritories() {
    game::TerritoryPatternRegistry::instance().registerPattern({"1",
                                                                {{1, 0},
                                                                 {1, -1},
                                                                 {0, -1},

                                                                 {-1, 0},
                                                                 {-1, 1},
                                                                 {0, 1}}});

    game::TerritoryPatternRegistry::instance().registerPattern({"2",
                                                                {{1, 0},
                                                                 {1, -1},
                                                                 {0, -1},

                                                                 {-1, 0},
                                                                 {-1, 1},
                                                                 {0, 1},

                                                                 {1, 1},
                                                                 {-1, -1}}});

    game::TerritoryPatternRegistry::instance().registerPattern({"3",
                                                                {
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

    game::TerritoryPatternRegistry::instance().registerPattern({"4",
                                                                {
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

    game::TerritoryPatternRegistry::instance().registerPattern({"5",
                                                                {
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
        {"6",
         {
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
