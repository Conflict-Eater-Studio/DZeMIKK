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
#include "ecs/serialize/prefabSerializer.h"
#include "input/input.h"
#include "map/HexCoord.h"
#include "map/HexPattern.h"
#include "map/ItemEntity.h"
#include "map/ItemEntityHealth.h"
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
#include <memory>
#include <utility>

#if DZEMIKK_DEV_TOOLS
#include <imgui.h>
#endif
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
#include "map/HexPattern.h"

#include <animation/animationstatemachine.h>
#include <ecs/components/animator.h>
#include <ecs/components/skinnedMeshRenderer.h>
#include <gameStateMachine.h>
#include <healthSystem.h>
#include <iostream>
#include "totem/totemManager.h"
#include <audio/sound.h>
#include <audio/audioManager.h>
#include "ecs/components/postProcessEffect.h"
#include "ecs/components/colorGradingEffect.h"
#include "ecs/components/antiAliasingEffect.h"

#include <random>
#include <ecs/components/light/pointLight.h>

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
    auto* assetManager = _engine->getAssetManager();
    auto* sceneManager = _engine->getSceneManager();

    setupSkybox();

    _mainScene = assetManager->get<dzemikk::Scene>("scenes/gameplay5.json");

    std::shared_ptr<dzemikk::Scene> sceneShared(_mainScene.get(), [](dzemikk::Scene*) {});
    sceneManager->loadScene(sceneShared);
    sceneManager->setActiveScene(sceneShared);

    setupMainCamera();
    setupUICamera();
    setupWorld();
    setupPlayer();
    registerDefaultTerritories();
    setupEnemies();
    // Setup Items and Totems ALWAYS after Enemies
    setupItems();
    setupTotems();

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
    _worldGO->addTag("World");
    auto* world = _worldGO->addComponent<game::World>(1);
    world->setGame(this);
    world->setModel(model);
    world->setMaterial(material);
    world->setMaterial2(material2);
    world->setResourceModel(resourceModel);
    world->setPlayer(_playerEntity);
    world->registerGenerator("full", [](int step, int maxSteps) { return 1.0F; });

    auto chunkMain1 = world->addChunk({.steps = 7});
    _chunkIds["chunkMain1"] = chunkMain1;

    auto chunkMain2 =
        world->addChunk({.parentChunkId = chunkMain1,
                         .steps = 12,
                         .dirFromParent = game::HexCoord::Direction::R0}); // connect chunk
    _chunkIds["chunkMain2"] = chunkMain2;

    auto chunkMain2Sub1 =
        world->addChunk({.parentChunkId = chunkMain2,
                         .steps = 15,
                         .dirFromParent = game::HexCoord::Direction::R330,
                         .unlockPattern = game::HexPattern({{-1, 1}, {0, 0}, {1, -1}},
                                                           game::HexPattern::Type::ATK, 1.2F)});
    _chunkIds["chunkMain2Sub1"] = chunkMain2Sub1;

    auto chunkMain2Sub2 = world->addChunk({.parentChunkId = chunkMain2Sub1,
                                           .steps = 11,
                                           .dirFromParent = game::HexCoord::Direction::R330});
    _chunkIds["chunkMain2Sub2"] = chunkMain2Sub2;

    auto chunkMain2Sub3 = world->addChunk({.parentChunkId = chunkMain2Sub2,
                                           .steps = 9,
                                           .dirFromParent = game::HexCoord::Direction::R0});
    _chunkIds["chunkMain2Sub3"] = chunkMain2Sub3;

    auto chunkMain3 = world->addChunk({.parentChunkId = chunkMain2,
                                       .steps = 17,
                                       .dirFromParent = game::HexCoord::Direction::R30});
    _chunkIds["chunkMain3"] = chunkMain3;

    auto chunkMain4 =
        world->addChunk({.parentChunkId = chunkMain3,
                         .steps = 15,
                         .dirFromParent = game::HexCoord::Direction::R30}); // connect chunk
    _chunkIds["chunkMain4"] = chunkMain4;

    auto chunkMain5 = world->addChunk({.parentChunkId = chunkMain4,
                                       .steps = 19,
                                       .dirFromParent = game::HexCoord::Direction::R330});
    _chunkIds["chunkMain5"] = chunkMain5;

    auto chunkMain6 = world->addChunk({.parentChunkId = chunkMain5,
                                       .steps = 15,
                                       .dirFromParent = game::HexCoord::Direction::R30});
    _chunkIds["chunkMain6"] = chunkMain6;

    auto chunkMain7 =
        world->addChunk({.parentChunkId = chunkMain6,
                         .steps = 24,
                         .dirFromParent = game::HexCoord::Direction::R0}); // connect chunk
    _chunkIds["chunkMain7"] = chunkMain7;

    auto chunkMain4Sub1 = world->addChunk({.parentChunkId = chunkMain4,
                                           .steps = 14,
                                           .dirFromParent = game::HexCoord::Direction::R30});
    _chunkIds["chunkMain4Sub1"] = chunkMain4Sub1;

    auto chunkMain7Sub1 = world->addChunk({.parentChunkId = chunkMain7,
                                           .steps = 22,
                                           .dirFromParent = game::HexCoord::Direction::R330});
    _chunkIds["chunkMain7Sub1"] = chunkMain7Sub1;

    auto chunkMain7Sub2 = world->addChunk({.parentChunkId = chunkMain7Sub1,
                                           .steps = 15,
                                           .dirFromParent = game::HexCoord::Direction::R0});
    _chunkIds["chunkMain7Sub2"] = chunkMain7Sub2;

    auto chunkMain7Sub3 = world->addChunk({.parentChunkId = chunkMain7Sub2,
                                           .steps = 17,
                                           .dirFromParent = game::HexCoord::Direction::R330});
    _chunkIds["chunkMain7Sub3"] = chunkMain7Sub3;

    auto chunkMain8 = world->addChunk({.parentChunkId = chunkMain7,
                                       .steps = 17,
                                       .dirFromParent = game::HexCoord::Direction::R30});
    _chunkIds["chunkMain8"] = chunkMain8;

    auto chunkMain9 = world->addChunk(
        {.parentChunkId = chunkMain8, .steps = 22, .dirFromParent = game::HexCoord::Direction::R0});
    _chunkIds["chunkMain9"] = chunkMain9;

    auto chunkMain10 = world->addChunk(
        {.parentChunkId = chunkMain9, .steps = 30, .dirFromParent = game::HexCoord::Direction::R0});
    _chunkIds["chunkMain10"] = chunkMain10;

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
        ImGui::Begin("Light Debug Tools");

        auto& reg = dzemikk::ComponentRegistry::get();

        static std::vector<dzemikk::DirectionalLight*> dir;
        static std::vector<dzemikk::PointLight*> point;
        static std::vector<dzemikk::SpotLight*> spot;

        reg.getEnabledComponents<dzemikk::DirectionalLight>(dir);
        reg.getEnabledComponents<dzemikk::PointLight>(point);
        reg.getEnabledComponents<dzemikk::SpotLight>(spot);

        // ===== UI =====
        ImGui::Text("Directional Lights: %d", (int)dir.size());
        ImGui::Text("Point Lights: %d", (int)point.size());
        ImGui::Text("Spot Lights: %d", (int)spot.size());

        ImGui::Separator();

        if (ImGui::Button("Spawn Random Point Light")) {
            for (int i = 0; i < 100; i++) {
                spawnRandomPointLight();
            }
        }

        ImGui::End();


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
    _playerGO = playerGO;
    playerGO->addTag("Player");

    auto* inventory = playerGO->addComponent<game::Inventory>();
    inventory->setGame(this);

    dzemikk::AnimationClip* clip = nullptr;
    auto skeleton =
        playerGO->getComponent<dzemikk::SkinnedMeshRenderer>()->getModel().get()->getSkeleton();
    clip = skeleton->getClip("forward_2_60");
    auto animator = playerGO->getComponent<dzemikk::Animator>();
    animator->getStateMachine()->getState("Idle")->setClip(clip);

    _playerEntity = playerGO->addComponent<game::PlayerEntity>();
    _playerEntity->setGame(this);
    _playerMovement = playerGO->addComponent<game::PlayerMovement>();
    _playerMovement->setPlayerEntity(_playerEntity);
    _playerMovement->setSpeed(0.25F);

    _playerMovement->setGame(this);

    animator->play("Idle");

    _hexGrid = _worldGO->getComponent<game::World>()->getGrid();
    _playerMovement->setHexGrid(_hexGrid);

    auto playerPatternStats = playerGO->addComponent<game::PlayerPatternStatsComponent>();

    auto patternComponent = playerGO->addComponent<game::PlayerPatternComponent>();
    patternComponent->setEngine(_engine);
    patternComponent->setGrid(_hexGrid);
    patternComponent->setPlayerEntity(_playerEntity);

    auto playerPanel = _mainScene.get()->findGameObjectByName("Player_Panel");
    auto combatPlayerPanel = playerPanel->addComponent<game::CombatUIPanel>(
        true, game::CombatUIPanel::Mode::AvailablePatterns);
    combatPlayerPanel->setPatternsComponent(patternComponent);
    combatPlayerPanel->setAssetManager(_engine->getAssetManager());
    combatPlayerPanel->setCanvas(playerPanel);

    auto playerHealthGO = _mainScene.get()
                              ->findGameObjectByName("Player_Avatar_Panel")
                              ->findDescendantByName("Health_Holder");
    playerHealthGO->addTag("PlayerHealthSystem");

    auto playerHealthSystem = playerHealthGO->addComponent<game::HealthSystem>();
    playerHealthSystem->setOwner(playerHealthGO);
    playerHealthSystem->setHealth(30.0F);
    playerHealthSystem->setMaxHealth(30.0F);
    playerHealthSystem->setTextRenderer(
        playerHealthGO->findChildByName("Text")->getComponent<dzemikk::UITextRenderer>());
}

void Game::setupEnemies() {
    auto* enemyManagerGO = _mainScene.get()->findGameObjectByName("EnemyManager");

    auto* enemyManager = enemyManagerGO->addComponent<game::EnemyManager>();

    enemyManager->setWorld(_worldGO->getComponent<game::World>());
    enemyManager->setAssetManager(_engine->getAssetManager());

    std::vector<game::EnemySpawnConfig> chunkMain1Config = {{
        .personality = game::EnemyPersonality::Aggressive,
        .type = game::EnemyType::Normal,
        .count = 1,
        .hp = 15,
        .territoryPattern = "1",
        .blocksChunks = {_chunkIds["chunkMain2"]},
    }};
    enemyManager->setSpawnConfig(_chunkIds["chunkMain1"], chunkMain1Config);

    std::vector<game::EnemySpawnConfig> chunkMain2Config = {{
        .personality = game::EnemyPersonality::Balanced,
        .type = game::EnemyType::Normal,
        .count = 1,
        .hp = 20,
        .territoryPattern = "2",
        .blocksChunks = {_chunkIds["chunkMain3"]},
    }};
    enemyManager->setSpawnConfig(_chunkIds["chunkMain2"], chunkMain2Config);

    std::vector<game::EnemySpawnConfig> chunkMain2Sub1Config = {
        {.personality = game::EnemyPersonality::Aggressive,
         .type = game::EnemyType::Special,
         .count = 1,
         .hp = 30,
         .territoryPattern = "3",
         .blocksChunks = {_chunkIds["chunkMain2Sub2"]}},
    };
    enemyManager->setSpawnConfig(_chunkIds["chunkMain2Sub1"], chunkMain2Sub1Config);

    std::vector<game::EnemySpawnConfig> chunkMain2Sub2Config = {
        {.personality = game::EnemyPersonality::Aggressive,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 20,
         .territoryPattern = "2",
         .blocksChunks = {_chunkIds["chunkMain2Sub3"]}},
    };
    enemyManager->setSpawnConfig(_chunkIds["chunkMain2Sub2"], chunkMain2Sub2Config);

    std::vector<game::EnemySpawnConfig> chunkMain3Config = {
        {.personality = game::EnemyPersonality::Aggressive,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 20,
         .territoryPattern = "2",
         .blocksChunks = {_chunkIds["chunkMain4"]}},
        {.personality = game::EnemyPersonality::Defensive,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 25,
         .territoryPattern = "3",
         .blocksChunks = {_chunkIds["chunkMain4"]}},
    };
    enemyManager->setSpawnConfig(_chunkIds["chunkMain3"], chunkMain3Config);

    std::vector<game::EnemySpawnConfig> chunkMain4Config = {
        {.personality = game::EnemyPersonality::Defensive,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 25,
         .territoryPattern = "3",
         .blocksChunks = {_chunkIds["chunkMain5"]}},
        {.personality = game::EnemyPersonality::Balanced,
         .type = game::EnemyType::Special,
         .count = 1,
         .hp = 35,
         .territoryPattern = "4",
         .blocksChunks = {_chunkIds["chunkMain4Sub1"]}},
    };
    enemyManager->setSpawnConfig(_chunkIds["chunkMain4"], chunkMain4Config);

    std::vector<game::EnemySpawnConfig> chunkMain5Config = {
        {.personality = game::EnemyPersonality::Balanced,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 30,
         .territoryPattern = "3",
         .blocksChunks = {_chunkIds["chunkMain6"]}},
        {.personality = game::EnemyPersonality::Aggressive,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 25,
         .territoryPattern = "3",
         .blocksChunks = {_chunkIds["chunkMain6"]}},
    };
    enemyManager->setSpawnConfig(_chunkIds["chunkMain5"], chunkMain5Config);

    std::vector<game::EnemySpawnConfig> chunkMain6Config = {
        {.personality = game::EnemyPersonality::Balanced,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 35,
         .territoryPattern = "4",
         .blocksChunks = {_chunkIds["chunkMain7"]}},
    };
    enemyManager->setSpawnConfig(_chunkIds["chunkMain6"], chunkMain6Config);

    std::vector<game::EnemySpawnConfig> chunkMain7Config = {
        {.personality = game::EnemyPersonality::Defensive,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 25,
         .territoryPattern = "3",
         .blocksChunks = {_chunkIds["chunkMain8"]}},
        {.personality = game::EnemyPersonality::Defensive,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 30,
         .territoryPattern = "3",
         .blocksChunks = {_chunkIds["chunkMain8"]}},
    };
    enemyManager->setSpawnConfig(_chunkIds["chunkMain7"], chunkMain7Config);

    std::vector<game::EnemySpawnConfig> chunkMain7Sub1Config = {
        {.personality = game::EnemyPersonality::Defensive,
         .type = game::EnemyType::Special,
         .count = 1,
         .hp = 40,
         .territoryPattern = "5",
         .blocksChunks = {_chunkIds["chunkMain7Sub2"]}},
    };
    enemyManager->setSpawnConfig(_chunkIds["chunkMain7Sub1"], chunkMain7Sub1Config);

    std::vector<game::EnemySpawnConfig> chunkMain7Sub2Config = {{
        .personality = game::EnemyPersonality::Defensive,
        .type = game::EnemyType::Normal,
        .count = 1,
        .hp = 35,
        .territoryPattern = "4",
        .blocksChunks = {_chunkIds["chunkMain7Sub3"]},
    }};
    enemyManager->setSpawnConfig(_chunkIds["chunkMain7Sub2"], chunkMain7Sub2Config);

    std::vector<game::EnemySpawnConfig> chunkMain8Config = {
        {.personality = game::EnemyPersonality::Aggressive,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 35,
         .territoryPattern = "4",
         .blocksChunks = {_chunkIds["chunkMain9"]}},
        {.personality = game::EnemyPersonality::Balanced,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 40,
         .territoryPattern = "5",
         .blocksChunks = {_chunkIds["chunkMain9"]}},
    };
    enemyManager->setSpawnConfig(_chunkIds["chunkMain8"], chunkMain8Config);

    std::vector<game::EnemySpawnConfig> chunkMain9Config = {
        {.personality = game::EnemyPersonality::Balanced,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 35,
         .territoryPattern = "4",
         .blocksChunks = {_chunkIds["chunkMain10"]}},
        {.personality = game::EnemyPersonality::Aggressive,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 30,
         .territoryPattern = "1",
         .blocksChunks = {_chunkIds["chunkMain10"]}},
    };
    enemyManager->setSpawnConfig(_chunkIds["chunkMain9"], chunkMain9Config);

    std::vector<game::EnemySpawnConfig> chunkMain10Config = {
        {.personality = game::EnemyPersonality::Aggressive,
         .type = game::EnemyType::Normal,
         .count = 1,
         .hp = 50,
         .territoryPattern = "6"},
    };
    enemyManager->setSpawnConfig(_chunkIds["chunkMain10"], chunkMain10Config);

    enemyManager->spawnEnemiesPerChunk();

    auto* enemyPatternComponent = enemyManagerGO->addComponent<game::EnemyPatternComponent>();
    auto* enemyPanel = _mainScene.get()->findGameObjectByName("Enemy_Panel");
    auto* combatEnamyPanel =
        enemyPanel->addComponent<game::CombatUIPanel>(false, game::CombatUIPanel::Mode::EnemyUsage);
    combatEnamyPanel->setPatternsComponent(enemyPatternComponent);
    combatEnamyPanel->setAssetManager(_engine->getAssetManager());
    combatEnamyPanel->setCanvas(enemyPanel);

    auto enemyHealthGO = _mainScene.get()
                             ->findGameObjectByName("Enemy_Avatar_Panel")
                             ->findDescendantByName("Health_Holder");

    auto enemyHealthSystem = enemyHealthGO->addComponent<game::HealthSystem>();
    enemyHealthSystem->setOwner(enemyHealthGO);
    enemyHealthSystem->setTextRenderer(
        enemyHealthGO->findChildByName("Text")->getComponent<dzemikk::UITextRenderer>());
}

void Game::setupItems() {
    if (_worldGO == nullptr || _worldGO->getComponent<game::World>() == nullptr) {
        return;
    }

    auto* world = _worldGO->getComponent<game::World>();

    // Heal Item steup
    auto healChunks = {"chunkMain2Sub2", "chunkMain3",     "chunkMain4Sub1",
                       "chunkMain7",     "chunkMain7Sub2", "chunkMain9"};
    for (const auto& id : healChunks) {
        world->addItem<game::ItemEntity::ItemType::Heal>(_chunkIds[id], 10.0F);
    }

    // Reveal Pattern Item setup
    auto revealPatternChunks = {"chunkMain2Sub1", "chunkMain2Sub3", "chunkMain4", "chunkMain4Sub1",
                                "chunkMain5",     "chunkMain6",     "chunkMain7", "chunkMain7Sub1",
                                "chunkMain7Sub3", "chunkMain9"};
    for (const auto& id : revealPatternChunks) {
        world->addItem<game::ItemEntity::ItemType::RevealPattern>(_chunkIds[id]);
    }

    // Reveal Hex Item setup
    auto revealPatternHex = {"chunkMain2", "chunkMain2Sub1", "chunkMain2Sub2", "chunkMain4",
                             "chunkMain6", "chunkMain7Sub1", "chunkMain7Sub2"};
    for (const auto& id : revealPatternChunks) {
        world->addItem<game::ItemEntity::ItemType::RevealHex>(_chunkIds[id]);
    }

    // Bonus Hex Item Setup
    game::HexPattern pat = game::HexPattern({{0, 0}}, game::HexPattern::Type::BONUSHEX);
    auto bonusHex = {"chunkMain2Sub1", "chunkMain2Sub3", "chunkMain2Sub3", "chunkMain4Sub1",
                     "chunkMain7",     "chunkMain7Sub1", "chunkMain7Sub3", "chunkMain7Sub3"};
    for (const auto& id : bonusHex) {
        world->addItem<game::ItemEntity::ItemType::BonusHex>(_chunkIds[id], pat);
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

void Game::setupTotems() {
    auto* go = _mainScene.get()->findGameObjectByName("TotemManager");

    auto* manager = go->addComponent<game::TotemManager>(2);

    manager->setWorld(_worldGO->getComponent<game::World>());

    manager->setAssetManager(_engine->getAssetManager());

    manager->setGame(this);

    manager->setSpawnConfig(_chunkIds["chunkMain2"],
                            {{.count = 1,
                              .pattern = game::HexPattern({{-1, 1}, {0, 0}, {1, -1}},
                                                          game::HexPattern::Type::ATK, 1.2F),
                              .prefabPath = "prefabs/totem/totem_container.prefab"}});

    manager->spawnTotemsPerChunk();

}