#pragma once 

#include "game.h"

#include "ecs/components/animator.h"
#include "animation/animationclip.h"
#include "animation/quaterniontrack.h"
#include "animation/vectortrack.h"
#include "assetManager/assetmanager.h"
#include "audio/sound.h"
#include "collisions/collisions.h"
#include "core/engine.h"
#include "core/window.h"
#include "ecs/components/camera.h"
#include "ecs/components/collider.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/ui/canvas.h"
#include "ecs/components/ui/colors.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/rectTransform.h"
#include "ecs/components/ui/uiButton.h"
#include "ecs/components/ui/uiCheckbox.h"
#include "ecs/components/ui/uiSlider.h"
#include "ecs/components/ui/uiTextRenderer.h"
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

#include "utils/perlin.h"
#include "scripts/world/world.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>


Game::Game(dzemikk::Engine* engine) : engine(engine) {}

void Game::start() {

    game::Perlin perlin(1);

    auto* assetManager = engine->getAssetManager();
    auto* sceneManager = engine->getSceneManager();

    auto scene = std::make_shared<dzemikk::Scene>();
    sceneManager->loadScene(scene);
    sceneManager->setActiveScene(scene);

    auto* cameraGO = scene->createGameObject("Camera");
    cameraGO->transform()->setPosition({0.0F, 100.0F, 100.0F});
    auto* camera = cameraGO->addComponent<dzemikk::Camera>();
    camera->lookAt({0.0F, 0.0F, 0.0F});
    engine->getRenderer()->setActiveSceneCamera(camera);

    auto* uiCameraGO = scene->createGameObject("UICamera");
    uiCameraGO->transform()->setPosition({0.0F, 0.0F, 1.0F});
    auto* uiCamera = uiCameraGO->addComponent<dzemikk::Camera>();
    uiCamera->setOrthographic(0.0F, 1920.0F, 0.0F, 1080.0F, -1.0F, 1.0F);
    engine->getRenderer()->setActiveUICamera(uiCamera);

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
    world->setModel(model);
    world->setMaterial(material);
    world->setMaterial2(material2);
    world->setEnemyModel(enemyModel);
    world->setResourceModel(resourceModel);
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

    engine->getRenderer()->setActiveSceneCamera(camera);
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

    engine->getRenderer()->setActiveUICamera(cameraUI);
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

            dzemikk::Collider* collider =
                engine->getCollisions()->raycast(engine->getRenderer()->getActiveSceneCamera(),
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