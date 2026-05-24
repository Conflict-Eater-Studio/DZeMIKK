#pragma once

#include "game.h"

#include "animation/animationclip.h"
#include "animation/animationmodule.h"
#include "animation/animationstatemachine.h"
#include "assetManager/assetmanager.h"
#include "audio/audioManager.h"
#include "audio/sound.h"
#include "collisions/collisions.h"
#include "core/engine.h"
#include "core/time.h"
#include "core/window.h"
#include "ecs/components/animator.h"
#include "ecs/components/camera.h"
#include "ecs/components/collider.h"
#include "ecs/components/light/directionalLight.h"
#include "ecs/components/light/pointLight.h"
#include "ecs/components/light/spotLight.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/skinnedMeshRenderer.h"
#include "ecs/components/ui/canvas.h"
#include "ecs/components/ui/gridLayout.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/rectTransform.h"
#include "ecs/components/ui/uiBuilder.h"
#include "ecs/components/ui/uiButton.h"
#include "ecs/components/ui/uiCheckbox.h"
#include "ecs/components/ui/uiSlider.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/scenemanager.h"
#include "input/input.h"
#include "map/HexCoord.h"
#include "map/PlayerEntity.h"
#include "renderer/cameraSystem.h"
#include "renderer/font.h"
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

struct EnemyInitContext {
    dzemikk::SkinnedMeshRenderer* renderer{};
    dzemikk::GameObject* go{};
    std::shared_ptr<dzemikk::AnimationStateMachine> sm;
    dzemikk::Animator* animator{};
};

void onEnemyModelLoaded(const dzemikk::AssetHandle<dzemikk::Model>& model, EnemyInitContext& ctx) {

    spdlog::info("OnEnemyModeLoaded");
    ctx.renderer->setModel(model);

    auto skeleton = model.get()->getSkeleton();

    dzemikk::AnimationClip* clip = nullptr;
    clip = skeleton->getClip("mixamo.com");

    auto* state = ctx.sm->addState("idle");
    state->setClip(clip);

    ctx.animator->play("idle");
}

dzemikk::DirectionalLight* sunLight = nullptr;
dzemikk::PointLight* pointLight = nullptr;
dzemikk::SpotLight* spotLight = nullptr;

struct ModelInitContext {
    dzemikk::MeshRenderer* renderer;
};

void onModelLoaded(dzemikk::AssetHandle<dzemikk::Model> model, ModelInitContext& ctx) {
    ctx.renderer->setModel(std::move(model));
}

void Game::spawnModel(dzemikk::Scene* scene, std::shared_ptr<dzemikk::Material> material,
                      const std::string& modelPath, const glm::vec3& position,
                      const glm::vec3& scale, const glm::quat& rotation) {
    auto* go = scene->createGameObject();

    go->transform()->setPosition(position);
    go->transform()->setScale(scale);
    go->transform()->setRotation(rotation);

    auto* meshR = go->addComponent<dzemikk::MeshRenderer>();
    meshR->setTransform(go->transform());
    meshR->setMaterial(0, std::move(material));

    ModelInitContext ctx(meshR);

    dzemikk::AssetManager::AssetTask<dzemikk::Model, ModelInitContext> task;
    task.context = ctx;
    task.onLoad = onModelLoaded;

    _engine->getAssetManager()->getAsync<dzemikk::Model>(modelPath, task);
}

void Game::newModels(const std::shared_ptr<dzemikk::Material>& m, dzemikk::Scene* scene) {
    spawnModel(scene, m, "models/czaszka.fbx", glm::vec3(-4.0F, 5.5F, 5.0F), glm::vec3(4.01F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/drezwo_niskie.fbx", glm::vec3(-5.0F, 5.5F, 5.0F), glm::vec3(1.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/drezwo_wysokie.fbx", glm::vec3(-6.5F, 5.5F, 5.0F), glm::vec3(1.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/kamyk_v1.fbx", glm::vec3(-7.5F, 5.5F, 5.0F), glm::vec3(4.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/kamyk_v2.fbx", glm::vec3(-8.5F, 5.5F, 5.0F), glm::vec3(4.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/krysztal_v1.fbx", glm::vec3(-9.5F, 5.5F, 5.0F), glm::vec3(4.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/krysztal_v2.fbx", glm::vec3(-10.5F, 5.5F, 5.0F), glm::vec3(4.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/krysztal_v3.fbx", glm::vec3(-11.5F, 5.5F, 5.0F), glm::vec3(4.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/krzak.fbx", glm::vec3(-13.0F, 5.5F, 5.0F), glm::vec3(1.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/lisc.fbx", glm::vec3(-14.0F, 5.5F, 5.0F), glm::vec3(4.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/muszla.fbx", glm::vec3(-15.F, 5.5F, 5.0F), glm::vec3(4.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/ognisko.fbx", glm::vec3(-17.F, 5.5F, 5.0F), glm::vec3(4.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/podest_kamien.fbx", glm::vec3(-19.F, 5.5F, 5.0F), glm::vec3(1.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/podest_modulo.fbx", glm::vec3(-21.F, 5.5F, 5.0F), glm::vec3(1.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/szkrzynka.fbx", glm::vec3(-23.F, 5.5F, 5.0F), glm::vec3(1.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/tipi.fbx", glm::vec3(-25.F, 5.5F, 5.0F), glm::vec3(1.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/tipi_z_galezami.fbx", glm::vec3(-27.F, 5.5F, 5.0F),
               glm::vec3(1.0F), glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/znak.fbx", glm::vec3(-29.F, 5.5F, 5.0F), glm::vec3(1.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/totem.fbx", glm::vec3(-32.F, 5.5F, 5.0F), glm::vec3(.2f),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    spawnModel(scene, m, "models/Baba.fbx", glm::vec3(-35.F, 5.5F, 5.0F), glm::vec3(1.0F),
               glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    // spawnModel(scene, m, "models/MainC(2).fbx", glm::vec3(-37.f, 5.5F, 5.0F), glm::vec3(1.0F),
    //          glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));
}

Game::Game(dzemikk::Engine* engine) : _engine(engine) {}

void Game::start() {

    game::Perlin perlin(1);

    auto* assetManager = _engine->getAssetManager();
    auto* sceneManager = _engine->getSceneManager();

    auto shaderC = _engine->getAssetManager()->get<dzemikk::Shader>("shaders/skinned");
    auto materialC = new dzemikk::Material();
    materialC->setShader(shaderC);

    auto shaderD = _engine->getAssetManager()->get<dzemikk::Shader>("shaders/skinned2");
    auto materialD = new dzemikk::Material();
    materialD->setShader(shaderD);

    auto skyboxShader = _engine->getAssetManager()->get<dzemikk::Shader>("shaders/skybox");

    SkyboxInitContext sCtx(skyboxShader, _engine->getRenderer(), _engine);
    dzemikk::AssetManager::AssetTask<dzemikk::Skybox, SkyboxInitContext> taskSk;
    taskSk.context = sCtx;
    taskSk.onLoad = onSkyboxLoad;
    _engine->getAssetManager()->getAsync("textures/Daylight Box_Pieces", taskSk);

    auto scene = std::make_shared<dzemikk::Scene>();
    sceneManager->loadScene(scene);
    sceneManager->setActiveScene(scene);

    auto* cameraGO = scene->createGameObject("Camera");
    cameraGO->transform()->setPosition({0.0F, 7.0F, 10.0F});
    auto* camera = cameraGO->addComponent<dzemikk::Camera>();
    camera->lookAt({0.0F, 2.0F, 0.0F});
    _engine->getRenderer()->getCameraSystem().setActiveSceneCamera(camera);

    auto* uiCameraGO = scene->createGameObject("UICamera");
    uiCameraGO->transform()->setPosition({0.0F, 0.0F, 1.0F});
    auto* uiCamera = uiCameraGO->addComponent<dzemikk::Camera>();
    uiCamera->setOrthographic(0.0F, 1920.0F, 0.0F, 1080.0F, -1.0F, 1.0F);
    _engine->getRenderer()->getCameraSystem().setActiveUICamera(uiCamera);

    auto* sunGO = scene->createGameObject("Sun");
    sunLight = sunGO->addComponent<dzemikk::DirectionalLight>();

    sunLight->setDirection(glm::normalize(glm::vec3(-0.5F, -1.0F, -0.3F)));
    sunLight->setColor(glm::vec3(1.0F, 0.95F, 0.8F));
    sunLight->setIntensity(0.9);

    auto shader = assetManager->get<dzemikk::Shader>("shaders/tile1");
    auto shader2 = assetManager->get<dzemikk::Shader>("shaders/tile2");
    auto material = std::make_shared<dzemikk::Material>();
    material->setShader(shader);
    auto material2 = std::make_shared<dzemikk::Material>();
    material2->setShader(shader2);

    auto model = assetManager->get<dzemikk::Model>("models/hex_wypukly.fbx");

    dzemikk::AssetHandle<dzemikk::Model> enemyModel =
        assetManager->getPrimitiveModel(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Capsule);

    dzemikk::AssetHandle<dzemikk::Model> resourceModel =
        assetManager->getPrimitiveModel(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Sphere);

    auto* rootGO = scene->createGameObject("Root");
    auto* worldGO = scene->createGameObject("World", rootGO);

    auto* playerGO = scene->createGameObject("Player", worldGO);
    auto* playerMesh = playerGO->addComponent<dzemikk::MeshRenderer>();
    _playerEntity = playerGO->addComponent<game::PlayerEntity>();
    playerMesh->setModel(enemyModel);
    playerMesh->setMaterial(0, material);
    playerMesh->setTransform(playerGO->transform());
    playerGO->transform()->setPosition({0.0F, 2.5F, 0.0F});

    auto* world = worldGO->addComponent<game::World>(1);

    world->setModel(model);
    world->setMaterial(material);
    world->setMaterial2(material2);
    world->setEnemyModel(enemyModel);
    world->setResourceModel(resourceModel);
    world->setPlayer(_playerEntity);

    world->registerGenerator("full", [](int step, int maxSteps) { return 1.0F; });

    auto c1 = world->addChunk({.steps = 4});

    auto c2 = world->addChunk(
        {.parentChunkId = c1, .steps = 8, .dirFromParent = game::HexCoord::Direction::R0});
    auto c3 = world->addChunk(
        {.parentChunkId = c2, .steps = 12, .dirFromParent = game::HexCoord::Direction::R0});
    auto c4 = world->addChunk(
        {.parentChunkId = c3, .steps = 16, .dirFromParent = game::HexCoord::Direction::R0});
    auto c5 = world->addChunk(
        {.parentChunkId = c4, .steps = 10, .dirFromParent = game::HexCoord::Direction::R0});

    auto c6 = world->addChunk(
        {.parentChunkId = c5, .steps = 12, .dirFromParent = game::HexCoord::Direction::R0});

    auto c7 = world->addChunk(
        {.parentChunkId = c6, .steps = 14, .dirFromParent = game::HexCoord::Direction::R0});

    auto c8 = world->addChunk(
        {.parentChunkId = c7, .steps = 16, .dirFromParent = game::HexCoord::Direction::R0});

    auto c3s1 = world->addChunk(
        {.parentChunkId = c3, .steps = 26, .dirFromParent = game::HexCoord::Direction::R60});
    auto c3s2 = world->addChunk(
        {.parentChunkId = c3, .steps = 8, .dirFromParent = game::HexCoord::Direction::R300});

    auto c3s2s1 = world->addChunk(
        {.parentChunkId = c3s2, .steps = 12, .dirFromParent = game::HexCoord::Direction::R300});

    std::ofstream out("./world.json");
    out << world->save().dump(4);
    out.close();

    // std::ifstream in("./world.json");
    // nlohmann::json worldData = nlohmann::json::parse(in);
    // world->load(worldData);

    _playerEntity->tryMove(world->getGrid()->at({0, 0}));

    auto* uiRootGO = scene->createGameObject("UI Root");
    auto* canvas = uiRootGO->addComponent<dzemikk::Canvas>();
    uiRootGO->rectTransform()->setSize({1920.0F, 1080.0F});
    auto quadMesh =
        assetManager->getPrimitiveMesh(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);
    auto quadShader = assetManager->get<dzemikk::Shader>("shaders/quad");
    auto quadMat = std::make_shared<dzemikk::Material>();
    quadMat->setShader(quadShader);

    auto font = assetManager->get<dzemikk::Font>("fonts/UncialAntiqua-Regular.ttf");

    /*

    // ====================== LAYOUT TEST UI ======================
    // Horizontal Layout (6 buttons)
    auto* horiGO = scene->createGameObject("HoriGO", uiRootGO);
    horiGO->rectTransform()->setSize({0.0F, 100.0F});
    horiGO->rectTransform()->setAnchorMin({0.0F, 0.5F});
    horiGO->rectTransform()->setAnchorMax({1.0F, 0.5F});
    horiGO->rectTransform()->setOffsetMin({10.0F, 0.0F});
    horiGO->rectTransform()->setOffsetMax({10.0F, 0.0F});
    auto* horiLayout = horiGO->addComponent<dzemikk::HorizontalLayout>();
    horiLayout->setSpacing(5.0F);
    horiLayout->setChildForceExpandHeight(true);

    for (int i = 0; i < 6; ++i) {
        auto btnParams = dzemikk::UIBuilder::UIButtonParams{.name = "HoriBtn" + std::to_string(i),
                                                            .size = {0.0F, 0.0F},
                                                            .text = "H" + std::to_string(i + 1),
                                                            .textFont = font,
                                                            .mesh = quadMesh,
                                                            .material = quadMat};
        dzemikk::UIBuilder::createButton(horiGO, btnParams);
    }
    horiLayout->rebuild();

    // Vertical Layout (6 buttons)
    auto* vertGO = scene->createGameObject("VertGO", uiRootGO);
    vertGO->rectTransform()->setSize({100.0F, 0.0F});
    vertGO->rectTransform()->setAnchorMin({0.5F, 0.0F});
    vertGO->rectTransform()->setAnchorMax({0.5F, 1.0F});
    vertGO->rectTransform()->setOffsetMin({0.0F, 10.0F});
    vertGO->rectTransform()->setOffsetMax({0.0F, 10.0F});
    auto* vertLayout = vertGO->addComponent<dzemikk::VerticalLayout>();
    vertLayout->setSpacing(5.0F);
    vertLayout->setChildForceExpandWidth(true);

    for (int i = 0; i < 6; ++i) {
        auto btnParams = dzemikk::UIBuilder::UIButtonParams{.name = "VertBtn" + std::to_string(i),
                                                            .size = {80.0F, 90.0F},
                                                            .text = "V" + std::to_string(i + 1),
                                                            .textFont = font,
                                                            .mesh = quadMesh,
                                                            .material = quadMat};
        dzemikk::UIBuilder::createButton(vertGO, btnParams);
    }
    vertLayout->rebuild();
    //
    // Grid Layout (6 buttons)
    auto* gridGO = scene->createGameObject("GridGO", uiRootGO);
    gridGO->rectTransform()->setAnchorMin({0.0F, 0.0F});
    gridGO->rectTransform()->setAnchorMax({1.0F, 1.0F});
    gridGO->rectTransform()->setOffsetMin({10.0F, 10.0F});
    gridGO->rectTransform()->setOffsetMax({10.0F, 10.0F});
    auto* gridLayout = gridGO->addComponent<dzemikk::GridLayout>();
    gridLayout->setStartCorner(dzemikk::LayoutStartCorner::UpperLeft);
    gridLayout->setCellSize({0.0F, 0.0F}); // dynamic: calculate from container size
    gridLayout->setSpacing({10.0F, 10.0F});
    gridLayout->setColumns(3);

    for (int i = 0; i < 5; ++i) {
        auto btnParams = dzemikk::UIBuilder::UIButtonParams{.name = "GridBtn" + std::to_string(i),
                                                            .size = {0.0F, 0.0F},
                                                            .text = "G" + std::to_string(i + 1),
                                                            .textFont = font,
                                                            .mesh = quadMesh,
                                                            .material = quadMat};
        dzemikk::UIBuilder::createButton(gridGO, btnParams);
    }
    gridLayout->rebuild();
    // ====================== END LAYOUT TEST ======================

    // Key input to switch layouts
    engine->getInput()->OnKeyPressed.addListener(
        [horiGO, vertGO, gridGO](dzemikk::KeyPressedEvent& event) {
            if (event.GetKeyCode() == GLFW_KEY_1) {
                horiGO->enabled(true);
                vertGO->enabled(false);
                gridGO->enabled(false);
            } else if (event.GetKeyCode() == GLFW_KEY_2) {
                horiGO->enabled(false);
                vertGO->enabled(true);
                gridGO->enabled(false);
            } else if (event.GetKeyCode() == GLFW_KEY_3) {
                horiGO->enabled(false);
                vertGO->enabled(false);
                gridGO->enabled(true);
            }
        });

    // Set initial layout visibility
    //horiGO->enabled(true);
    horiGO->enabled(false);
    vertGO->enabled(false);
    gridGO->enabled(false);
    */

    auto* dropdownGO = dzemikk::UIBuilder::createDropdown(
        uiRootGO, {
                      .name = "Test Dropdown",
                      .position = {(1920.0F / 4.0F) - 100.0F, 1080.0F - 150.0F},
                      .size = {400.0F, 70.0F},
                      .options =
                          {
                              {.text = "Option 1", .value = "opt1"},
                              {.text = "Option 2", .value = "opt2"},
                              {.text = "Option 3", .value = "opt3"},
                              {.text = "Option 4", .value = "opt4"},
                              {.text = "Option 5", .value = "opt5"},
                              {.text = "Option 6", .value = "opt6"},
                          },
                      .optionHeight = 70.0F,
                      .maxVisibleOptions = 3,
                      .textFont = font,
                      .bgMesh = quadMesh,
                      .arrowMesh = quadMesh,
                      .optionMesh = quadMesh,
                      .optionsBgMesh = quadMesh,
                      .bgMat = quadMat,
                      .arrowMat = quadMat,
                      .optionMat = quadMat,
                      .optionsBgMat = quadMat,
                  });

    auto* dropdown = dropdownGO->getComponent<dzemikk::UIDropdown>();

    _enemyGO = scene->createGameObject();
    _enemyGO->transform()->setPosition(glm::vec3(2.0F, 2.5F, 5.0F));
    _enemyGO->transform()->setScale(glm::vec3(0.01F, 0.01F, 0.01F));
    auto* enemyMeshR = _enemyGO->addComponent<dzemikk::SkinnedMeshRenderer>();
    // auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/Body Block.fbx");
    //  enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/Rumba Dancing.fbx");
    // auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/Flair(1).fbx");
    // auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/Dancing Twerk.fbx");
    // auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/szamankaanim.fbx");
    //  auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/MainC.fbx");
    //  auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/cooper.fbx");
    enemyMeshR->setTransform(_enemyGO->transform());
    enemyMeshR->setMaterial(0, materialC);
    enemyMeshR->setMaterial(1, materialD);
    auto sm = std::make_shared<dzemikk::AnimationStateMachine>();
    auto* animator = _enemyGO->addComponent<dzemikk::Animator>();
    _engine->getAnimationModule()->registerAnimator(animator);
    animator->setStateMachine(sm);

    EnemyInitContext ctx{.renderer = enemyMeshR, .go = _enemyGO, .sm = sm, .animator = animator};

    dzemikk::AssetManager::AssetTask<dzemikk::Model, EnemyInitContext> task;
    task.context = ctx;
    task.onLoad = onEnemyModelLoaded;
    _engine->getAssetManager()->getAsync("models/Dancing Twerk.fbx", task);

    setupInputCallbacks();
    newModels(material, scene.get());

    // --- Sound test ---
    auto* audio = _engine->getAudioManager();

    auto sound = _engine->getAssetManager()->get<dzemikk::Sound>(
        "audio/prime_przygodowka (loop, ale przyjemny).wav");
    _engine->getInput()->OnKeyPressed.addListener([audio, &sound](dzemikk::KeyPressedEvent& event) {
        if (event.GetKeyCode() == GLFW_KEY_SPACE) {
            spdlog::info("Playing sound");
            audio->play(*sound.get(), dzemikk::AudioManager::SoundType::Music);
        }
    });

    // --- To save your ears :)
    audio->getMasterGroup()->setVolume(0.5F);

    _engine->start();
}

void Game::setupSkybox() {
    auto skybox = _engine->getAssetManager()->get<dzemikk::Skybox>("textures/Daylight Box_Pieces");
    skybox.get()->setShader(_engine->getAssetManager()->get<dzemikk::Shader>("shaders/skybox"));
    _engine->getRenderer()->setSkybox(skybox);
}

void Game::setupMainCamera() {
    auto* cameraGO = _mainScene->createGameObject();
    cameraGO->transform()->setPosition(glm::vec3(1.5F, 1.5F, 3.0F));

    auto* camera = cameraGO->addComponent<dzemikk::Camera>();
    camera->lookAt(glm::vec3(0.0F, 0.0F, 0.0F));

    _engine->getRenderer()->getCameraSystem().setActiveSceneCamera(camera);
}

void Game::setupMaterials() {
    auto shaderA = _engine->getAssetManager()->get<dzemikk::Shader>("shaders/tile1");
    _materialA = new dzemikk::Material();
    _materialA->setShader(shaderA);

    auto shaderB = _engine->getAssetManager()->get<dzemikk::Shader>("shaders/tile2");
    _materialB = new dzemikk::Material();
    _materialB->setShader(shaderB);

    auto quadShader = _engine->getAssetManager()->get<dzemikk::Shader>("shaders/quad");
    _quadMaterial = new dzemikk::Material();
    _quadMaterial->setShader(quadShader);
}

void Game::setupWorld() {}

void Game::setupChest() {
    auto* chestGO = _mainScene->createGameObject();
    chestGO->transform()->setPosition(glm::vec3(-4.0F, 2.5F, 0.0F));
    chestGO->transform()->setRotation(glm::angleAxis(glm::radians(-90.0F), glm::vec3(1, 0, 0)));

    auto* chestMeshR = chestGO->addComponent<dzemikk::MeshRenderer>();
    auto chestMesh = _engine->getAssetManager()->get<dzemikk::Model>("models/skrzynia.fbx");

    chestMeshR->setModel(chestMesh);
    chestMeshR->setTransform(chestGO->transform());
    // chestMeshR->setMaterial(0, materialA);
}

void Game::setupEnemy() {
    auto* enemyGO = _mainScene->createGameObject();
    enemyGO->transform()->setPosition(glm::vec3(2.0F, 1.5F, 0.0F));
    enemyGO->transform()->setScale(glm::vec3(0.01F, 0.01F, 0.01F));

    auto* enemyMeshR = enemyGO->addComponent<dzemikk::MeshRenderer>();
    auto enemyMesh = _engine->getAssetManager()->get<dzemikk::Model>("models/Body Block.fbx");

    enemyMeshR->setModel(enemyMesh);
    enemyMeshR->setTransform(enemyGO->transform());
    // enemyMeshR->setMaterial(0, materialA);
    // enemyMeshR->setMaterial(1, materialB);
}

void Game::setupUICamera() {
    auto* cameraUIGO = _mainScene->createGameObject();
    cameraUIGO->transform()->setPosition(glm::vec3(0.0F, 0.0F, 1.0F));

    auto* cameraUI = cameraUIGO->addComponent<dzemikk::Camera>();
    cameraUI->setOrthographic(0.0F, 1920.0F, 0.0F, 1080.0F, -1.0F, 1.0F);

    _engine->getRenderer()->getCameraSystem().setActiveUICamera(cameraUI);
}

void Game::setupUI() {
    auto* canvasGo = _mainScene->createGameObject("Canvas");
    auto* canvas = canvasGo->addComponent<dzemikk::Canvas>();
    (void)canvas;

    auto* canvasRect = canvasGo->rectTransform();
    canvasRect->setSize({1920.0F, 1080.0F});

    setupButton(canvasGo);
    setupSlider(canvasGo);
    setupCheckbox(canvasGo);
}

void Game::setupButton(dzemikk::GameObject* canvasGo) {}

void Game::setupSlider(dzemikk::GameObject* canvasGo) {}

void Game::setupCheckbox(dzemikk::GameObject* canvasGo) {}

void Game::setupAudio() {
    FMOD::System* system = nullptr;
    FMOD::System_Create(&system);
    system->init(512, FMOD_INIT_NORMAL, nullptr);

    _engine->getAssetManager()->setFMODSystem(system);

    auto sound = _engine->getAssetManager()->get<dzemikk::Sound>("audio/prime_wznoszeniePol.wav");
}

void Game::setupInputCallbacks() {

    static dzemikk::MeshRenderer* lastHitRenderer = nullptr;
    static auto lastHitColor = glm::vec4(1.0F);

    _engine->SetUserUpdateCallback([this]() {
        if (!_engine || !_engine->getInput()) {
            return;
        }

        // ================= GAMEPAD =================

        glm::vec3 pos = _enemyGO->transform()->getPosition();

        float speed = 5.0F * _engine->getTime()->getDeltaTime();

        if (_engine->getInput()->IsGamepadConnected(GLFW_JOYSTICK_1)) {

            float axisX =
                _engine->getInput()->GetGamepadAxis(GLFW_JOYSTICK_1, GLFW_GAMEPAD_AXIS_LEFT_X);

            float axisY =
                _engine->getInput()->GetGamepadAxis(GLFW_JOYSTICK_1, GLFW_GAMEPAD_AXIS_LEFT_Y);

            if (std::abs(axisX) > 0.1F) {
                pos.x += axisX * speed;
            }

            if (std::abs(axisY) > 0.1F) {
                pos.z += axisY * speed;
            }
        }

        _enemyGO->transform()->setPosition(pos);

        // ================= RAYCAST =================

        int windowWidth = 0;
        int windowHeight = 0;

        glfwGetWindowSize(_engine->getWindow()->nativeHandle(), &windowWidth, &windowHeight);

        dzemikk::Collider* collider = _engine->getCollisions()->raycast(
            _engine->getRenderer()->getCameraSystem().getActiveSceneCamera(),
            _engine->getInput()->GetMousePosition(), static_cast<float>(windowWidth),
            static_cast<float>(windowHeight));

        dzemikk::MeshRenderer* currentRenderer = nullptr;

        if (collider) {
            currentRenderer = collider->getOwner()->getComponent<dzemikk::MeshRenderer>();

            if (_engine->getInput()->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                auto* wh = collider->getOwner()->getComponent<game::WorldHex>();
                if (wh != nullptr && wh->getHexCell() != nullptr) {
                    _playerEntity->tryMove(wh->getHexCell());
                }
            }
        }

        if (currentRenderer != lastHitRenderer) {

            if (lastHitRenderer && lastHitRenderer->isValid()) {
                lastHitRenderer->setColor(lastHitColor);
            }

            if (currentRenderer) {
                lastHitColor = currentRenderer->getColor();
                currentRenderer->setColor(glm::vec4(1.0F, 0.0F, 0.0F, 1.0F));
            }

            lastHitRenderer = currentRenderer;
        }
    });
}