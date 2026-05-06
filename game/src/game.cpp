#pragma once 

#include "game.h"

#include "ecs/components/animator.h"
#include "animation/animationclip.h"
#include "animation/quaterniontrack.h"
#include "animation/vectortrack.h"
#include "animation/animationstatemachine.h"
#include "animation/animationmodule.h"
#include "assetManager/assetmanager.h"
#include "audio/sound.h"
#include "collisions/collisions.h"
#include "core/engine.h"
#include "core/window.h"
#include "ecs/components/camera.h"
#include "ecs/components/collider.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/skinnedMeshRenderer.h"
#include "ecs/components/ui/canvas.h"
#include "ecs/components/ui/colors.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/rectTransform.h"
#include "ecs/components/ui/uiButton.h"
#include "ecs/components/ui/uiCheckbox.h"
#include "ecs/components/ui/uiSlider.h"
#include "ecs/components/ui/uiTextRenderer.h"
#include "ecs/components/ui/gridLayout.h"
#include "ecs/components/ui/horizontalLayout.h"
#include "ecs/components/ui/uiActionRegistry.h"
#include "ecs/components/ui/uiBuilder.h"
#include "ecs/components/ui/uiEvent.h"
#include "ecs/components/ui/verticalLayout.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/scenemanager.h"
#include "input/input.h"
#include "renderer/font.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/model.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "renderer/cameraSystem.h"

#include "utils/perlin.h"
#include "scripts/world/world.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

struct SkyboxInitContext {
    dzemikk::AssetHandle<dzemikk::Shader> shader;
    dzemikk::Renderer* renderer;
    dzemikk::Engine* engine;
};

void onSkyboxLoad(dzemikk::AssetHandle<dzemikk::Skybox> skybox, SkyboxInitContext& ctx) {
    auto skyboxShader = ctx.engine->getAssetManager()->get<dzemikk::Shader>("shaders/skybox");
    skybox.get()->setShader(skyboxShader);
    ctx.renderer->setSkybox(skybox);
}

struct EnemyInitContext {
    dzemikk::SkinnedMeshRenderer* renderer;
    dzemikk::GameObject* go;
    std::shared_ptr<dzemikk::AnimationStateMachine> sm;
    dzemikk::Animator* animator;
};

void onEnemyModelLoaded(dzemikk::AssetHandle<dzemikk::Model> model, EnemyInitContext& ctx) {

    spdlog::info("OnEnemyModeLoaded");
    ctx.renderer->setModel(model);

    auto skeleton = model.get()->getSkeleton();

    dzemikk::AnimationClip* clip = nullptr;
    clip = skeleton->getClip("mixamo.com");

    auto state = ctx.sm->addState("idle"); 
    state->setClip(clip);

    ctx.animator->play("idle");
}

Game::Game(dzemikk::Engine* engine) : engine(engine) {}

void Game::start() {

    game::Perlin perlin(1);

    auto* assetManager = engine->getAssetManager();
    auto* sceneManager = engine->getSceneManager();

    auto shaderC = engine->getAssetManager()->get<dzemikk::Shader>("shaders/skinned");
    auto materialC = new dzemikk::Material();
    materialC->setShader(shaderC);

    auto shaderD = engine->getAssetManager()->get<dzemikk::Shader>("shaders/skinned2");
    auto materialD = new dzemikk::Material();
    materialD->setShader(shaderD);

    auto skyboxShader = engine->getAssetManager()->get<dzemikk::Shader>("shaders/skybox");

    SkyboxInitContext sCtx(skyboxShader, engine->getRenderer(), engine);
    dzemikk::AssetManager::AssetTask<dzemikk::Skybox, SkyboxInitContext> taskSk;
    taskSk.context = sCtx;
    taskSk.onLoad = onSkyboxLoad;
    engine->getAssetManager()->getAsync("textures/Daylight Box_Pieces", taskSk);

    auto scene = std::make_shared<dzemikk::Scene>();
    sceneManager->loadScene(scene);
    sceneManager->setActiveScene(scene);

    auto* cameraGO = scene->createGameObject("Camera");
    cameraGO->transform()->setPosition({0.0F, 7.0F, 10.0F});
    auto* camera = cameraGO->addComponent<dzemikk::Camera>();
    camera->lookAt({0.0F, 2.0F, 0.0F});
    engine->getRenderer()->getCameraSystem().setActiveSceneCamera(camera);

    auto* uiCameraGO = scene->createGameObject("UICamera");
    uiCameraGO->transform()->setPosition({0.0F, 0.0F, 1.0F});
    auto* uiCamera = uiCameraGO->addComponent<dzemikk::Camera>();
    uiCamera->setOrthographic(0.0F, 1920.0F, 0.0F, 1080.0F, -1.0F, 1.0F);
    engine->getRenderer()->getCameraSystem().setActiveUICamera(uiCamera);

    auto shader = assetManager->get<dzemikk::Shader>("shaders/tile1");
    auto shader2 = assetManager->get<dzemikk::Shader>("shaders/tile2");
    auto material = std::make_shared<dzemikk::Material>();
    material->setShader(shader);
    auto material2 = std::make_shared<dzemikk::Material>();
    material2->setShader(shader2);

    auto model = assetManager->get<dzemikk::Model>("models/pole.fbx");

    dzemikk::AssetHandle<dzemikk::Model> enemyModel =
        assetManager->getPrimitiveModel(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Capsule);

    dzemikk::AssetHandle<dzemikk::Model> resourceModel =
        assetManager->getPrimitiveModel(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Sphere);

    auto* rootGO = scene->createGameObject("Root");
    auto* worldGO = scene->createGameObject("World", rootGO);

    auto* world = worldGO->addComponent<game::World>(
        1, std::vector<std::tuple<int, int, std::vector<game::HexCoord::Direction>>>{
               {4, 6, {}},
               {6, 8, {}},
               {8, 10, {}},
               {10, 12, {game::HexCoord::Direction::R60}},
               {12, 14, {game::HexCoord::Direction::R300}},
               {14, 16, {}},
               {16, 18, {game::HexCoord::Direction::R60, game::HexCoord::Direction::R300}},
           });

    /*
    auto* world = worldGO->addComponent<game::World>(
        1, std::vector<std::tuple<int, int, std::vector<game::HexCoord::Direction>>>{
               {4, 6, {}},
               {6, 8, {}},
               {8, 10, {}},
               {10, 12, {game::HexCoord::Direction::R60}},
               {12, 14, {game::HexCoord::Direction::R300}},
               {14, 16, {}},
               {16, 18, {game::HexCoord::Direction::R60, game::HexCoord::Direction::R300}},

               {18, 20, {}},
               {20, 22, {game::HexCoord::Direction::R0}},
               {22, 24, {game::HexCoord::Direction::R60}},
               {24, 26, {game::HexCoord::Direction::R120}},
               {26, 28, {game::HexCoord::Direction::R300}},
               {28, 30, {}},

               {30, 34, {game::HexCoord::Direction::R60, game::HexCoord::Direction::R300}},
               {34, 38, {}},
               {38, 42, {game::HexCoord::Direction::R0, game::HexCoord::Direction::R120}},

               {42, 46, {game::HexCoord::Direction::R60}},
               {46, 50, {}},

               {50, 55, {game::HexCoord::Direction::R300}},
               {55, 60, {game::HexCoord::Direction::R60, game::HexCoord::Direction::R0}},
           });
        */
    world->setModel(model);
    world->setMaterial(material);
    world->setMaterial2(material2);
    world->setEnemyModel(enemyModel);
    world->setResourceModel(resourceModel);

    auto* uiRootGO = scene->createGameObject("UI Root");
    auto* canvas = uiRootGO->addComponent<dzemikk::Canvas>();
    uiRootGO->rectTransform()->setSize({1920.0F, 1080.0F});
    auto quadMesh = assetManager->getPrimitiveMesh(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);
    auto quadShader = assetManager->get<dzemikk::Shader>("shaders/quad");
    auto quadMat = std::make_shared<dzemikk::Material>();
    quadMat->setShader(quadShader);

    auto font = assetManager->get<dzemikk::Font>("fonts/UncialAntiqua-Regular.ttf");

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

    // Grid Layout (6 buttons)
    auto* gridGO = scene->createGameObject("GridGO", uiRootGO);
    gridGO->rectTransform()->setAnchorMin({0.0F, 0.0F});
    gridGO->rectTransform()->setAnchorMax({1.0F, 1.0F});
    gridGO->rectTransform()->setOffsetMin({10.0F, 10.0F});
    gridGO->rectTransform()->setOffsetMax({10.0F, 10.0F});
    auto* gridLayout = gridGO->addComponent<dzemikk::GridLayout>();
    gridLayout->setCellSize({0.0F, 0.0F}); // dynamic: calculate from container size
    gridLayout->setSpacing({10.0F, 10.0F});
    gridLayout->setColumns(3);

    for (int i = 0; i < 6; ++i) {
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

    auto enemyGO = scene->createGameObject();
    enemyGO->transform()->setPosition(glm::vec3(2.0f, 2.5f, 5.0f));
    enemyGO->transform()->setScale(glm::vec3(0.01f, 0.01f, .01f));
    auto enemyMeshR = enemyGO->addComponent<dzemikk::SkinnedMeshRenderer>();
    // auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/Body Block.fbx");
    //  enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/Rumba Dancing.fbx");
    // auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/Flair(1).fbx");
    // auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/Dancing Twerk.fbx");
    // auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/szamankaanim.fbx");
    //  auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/MainC.fbx");
    //  auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/cooper.fbx");
    enemyMeshR->setTransform(enemyGO->transform());
    enemyMeshR->setMaterial(0, materialC);
    enemyMeshR->setMaterial(1, materialD);
    auto sm = std::make_shared<dzemikk::AnimationStateMachine>();
    auto animator = enemyGO->addComponent<dzemikk::Animator>();
    engine->getAnimationModule()->registerAnimator(animator);
    animator->setStateMachine(sm);

    EnemyInitContext ctx{enemyMeshR, enemyGO, sm, animator};

    dzemikk::AssetManager::AssetTask<dzemikk::Model, EnemyInitContext> task;
    task.context = ctx;
    task.onLoad = onEnemyModelLoaded;
    engine->getAssetManager()->getAsync("models/Dancing Twerk.fbx", task);
}
void Game::setupScene() {
    mainScene = std::make_shared<dzemikk::Scene>();
    engine->getSceneManager()->loadScene(mainScene);
    engine->getSceneManager()->setActiveScene(mainScene);
}

void Game::setupSkybox() {
    auto skybox = engine->getAssetManager()->get<dzemikk::Skybox>("textures/Daylight Box_Pieces");
    skybox.get()->setShader(engine->getAssetManager()->get<dzemikk::Shader>("shaders/skybox"));
    engine->getRenderer()->setSkybox(skybox);
}

void Game::setupMainCamera() {
    auto cameraGO = mainScene->createGameObject();
    cameraGO->transform()->setPosition(glm::vec3(1.5f, 1.5f, 3.0f));

    auto camera = cameraGO->addComponent<dzemikk::Camera>();
    camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    engine->getRenderer()->getCameraSystem().setActiveSceneCamera(camera);
}

void Game::setupMaterials() {
    auto shaderA = engine->getAssetManager()->get<dzemikk::Shader>("shaders/tile1");
    materialA = new dzemikk::Material();
    materialA->setShader(shaderA);

    auto shaderB = engine->getAssetManager()->get<dzemikk::Shader>("shaders/tile2");
    materialB = new dzemikk::Material();
    materialB->setShader(shaderB);

    auto quadShader = engine->getAssetManager()->get<dzemikk::Shader>("shaders/quad");
    quadMaterial = new dzemikk::Material();
    quadMaterial->setShader(quadShader);
}

void Game::setupWorld() {

}

void Game::setupChest() {
    auto chestGO = mainScene->createGameObject();
    chestGO->transform()->setPosition(glm::vec3(-4.0f, 2.5f, 0.0f));
    chestGO->transform()->setRotation(glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0)));

    auto chestMeshR = chestGO->addComponent<dzemikk::MeshRenderer>();
    auto chestMesh = engine->getAssetManager()->get<dzemikk::Model>("models/skrzynia.fbx");

    chestMeshR->setModel(chestMesh);
    chestMeshR->setTransform(chestGO->transform());
    chestMeshR->setMaterial(0, materialA);
}

void Game::setupEnemy() {
    auto enemyGO = mainScene->createGameObject();
    enemyGO->transform()->setPosition(glm::vec3(2.0f, 1.5f, 0.0f));
    enemyGO->transform()->setScale(glm::vec3(.01f, .01f, 0.01f));

    auto enemyMeshR = enemyGO->addComponent<dzemikk::MeshRenderer>();
    auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/Body Block.fbx");

    enemyMeshR->setModel(enemyMesh);
    enemyMeshR->setTransform(enemyGO->transform());
    enemyMeshR->setMaterial(0, materialA);
    enemyMeshR->setMaterial(1, materialB);
}

void Game::setupUICamera() {
    auto cameraUIGO = mainScene->createGameObject();
    cameraUIGO->transform()->setPosition(glm::vec3(0.0f, 0.0f, 1.0f));

    auto cameraUI = cameraUIGO->addComponent<dzemikk::Camera>();
    cameraUI->setOrthographic(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f);

    engine->getRenderer()->getCameraSystem().setActiveUICamera(cameraUI);
}

void Game::setupUI() {
    auto* canvasGo = mainScene->createGameObject("Canvas");
    auto* canvas = canvasGo->addComponent<dzemikk::Canvas>();
    (void)canvas;

    auto* canvasRect = canvasGo->rectTransform();
    canvasRect->setSize({1920.0F, 1080.0F});

    setupButton(canvasGo);
    setupSlider(canvasGo);
    setupCheckbox(canvasGo);
}

void Game::setupButton(dzemikk::GameObject* canvasGo) {
}

void Game::setupSlider(dzemikk::GameObject* canvasGo) {

}

void Game::setupCheckbox(dzemikk::GameObject* canvasGo) {

}

void Game::setupAudio() {
    FMOD::System* system;
    FMOD::System_Create(&system);
    system->init(512, FMOD_INIT_NORMAL, nullptr);

    engine->getAssetManager()->setFMODSystem(system);

    auto sound = engine->getAssetManager()->get<dzemikk::Sound>("audio/prime_wznoszeniePol.wav");
}

void Game::setupInputCallbacks() {
    engine->SetUserUpdateCallback([this]() {
        if (!engine || !engine->getInput() || !playerGO) {
            return;
        }

        if (engine->getInput()->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            int windowWidth = 0;
            int windowHeight = 0;

            glfwGetWindowSize(engine->getWindow()->nativeHandle(), &windowWidth, &windowHeight);

            dzemikk::Collider* collider = engine->getCollisions()->raycast(
                engine->getRenderer()->getCameraSystem().getActiveSceneCamera(),
                                                 engine->getInput()->GetMousePosition(),
                                                 windowWidth,
                                                 windowHeight);

            if (collider) {
                auto hit = collider->getOwner()->getComponent<dzemikk::Transform>();
                collider->getOwner()
                    ->getComponent<dzemikk::MeshRenderer>()
                    ->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

                glm::vec3 position = hit->getPosition();
                position.y += 1.5f;

                playerGO->transform()->setPosition(position);
            }
        }
    });
}