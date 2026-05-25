#pragma once

#include "game.h"

#include "assetManager/assetmanager.h"
#include "collisions/collisions.h"
#include "core/engine.h"
#include "core/time.h"
#include "core/window.h"
#include "ecs/components/camera.h"
#include "ecs/components/collider.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/scenemanager.h"
#include "input/input.h"
#include "map/HexCoord.h"
#include "map/PlayerEntity.h"
#include "playerMovement.h"
#include "renderer/cameraSystem.h"
#include "renderer/material.h"
#include "renderer/model.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "scripts/world/world.h"
#include "scripts/world/worldHex.h"
#include "utils/perlin.h"
#include "camera/cameraController.h"
#include "ecs/components/ui/uiActionRegistry.h"

#include <GLFW/glfw3.h>
#include <memory>
#include <utility>

#if DZEMIKK_DEV_TOOLS
#include <imgui.h>
#endif
#include <gameStateMachine.h>
#include "stateMachine/explorationState.h"
#include "stateMachine/combatState.h"

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


    _mainScene = assetManager->get<dzemikk::Scene>("scenes/gameplay2.json");

    std::shared_ptr<dzemikk::Scene> sceneShared(_mainScene.get(), [](dzemikk::Scene*) {});
    sceneManager->loadScene(sceneShared);
    sceneManager->setActiveScene(sceneShared);

    setupMainCamera();
    setupUICamera();
    setupWorld();
    setupPlayer();

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

    auto c2 = world->addChunk(
        {.parentChunkId = c1, .steps = 12, .dirFromParent = game::HexCoord::Direction::R0}); //connect chunk

    auto c3 = world->addChunk(
        {.parentChunkId = c2, .steps = 15, .dirFromParent = game::HexCoord::Direction::R330});

    auto c4 = world->addChunk(
        {.parentChunkId = c3, .steps = 11, .dirFromParent = game::HexCoord::Direction::R330});

    auto c5 = world->addChunk(
        {.parentChunkId = c4, .steps = 9, .dirFromParent = game::HexCoord::Direction::R0});

    auto c6 = world->addChunk(
        {.parentChunkId = c2, .steps = 17, .dirFromParent = game::HexCoord::Direction::R30});

    auto c7 = world->addChunk(
        {.parentChunkId = c6, .steps = 15, .dirFromParent = game::HexCoord::Direction::R30}); //connect chunk

    auto c8 = world->addChunk(
        {.parentChunkId = c7, .steps = 19, .dirFromParent = game::HexCoord::Direction::R330});

    auto c9 = world->addChunk(
        {.parentChunkId = c8, .steps = 15, .dirFromParent = game::HexCoord::Direction::R30});

    auto c10 = world->addChunk(
        {.parentChunkId = c9, .steps = 24, .dirFromParent = game::HexCoord::Direction::R0}); //connect chunk

    auto c11 = world->addChunk(
        {.parentChunkId = c7, .steps = 14, .dirFromParent = game::HexCoord::Direction::R30});

    auto c12 = world->addChunk(
        {.parentChunkId = c10, .steps = 22, .dirFromParent = game::HexCoord::Direction::R330});

    auto c13 = world->addChunk(
        {.parentChunkId = c12, .steps = 15, .dirFromParent = game::HexCoord::Direction::R0});

    auto c14 = world->addChunk(
        {.parentChunkId = c13, .steps = 17, .dirFromParent = game::HexCoord::Direction::R330});

    auto c15 = world->addChunk(
        {.parentChunkId = c10, .steps = 17, .dirFromParent = game::HexCoord::Direction::R30});

    auto c16 = world->addChunk(
        {.parentChunkId = c15, .steps = 22, .dirFromParent = game::HexCoord::Direction::R0});

    auto c17 = world->addChunk(
        {.parentChunkId = c16, .steps = 30, .dirFromParent = game::HexCoord::Direction::R0});

    /*

    auto c3s2 = world->addChunk(
        {.parentChunkId = c3, .steps = 8, .dirFromParent = game::HexCoord::Direction::R300});

    auto c3s2s1 = world->addChunk(
        {.parentChunkId = c3s2, .steps = 12, .dirFromParent = game::HexCoord::Direction::R300});

    */
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
    
    _engine->SetUserUpdateCallback([this]() {
        if (!_engine || !_engine->getInput()) {
            return;
        }

        int windowWidth = 0;
        int windowHeight = 0;

        glfwGetWindowSize(_engine->getWindow()->nativeHandle(), &windowWidth, &windowHeight);

        dzemikk::Collider* collider = _engine->getCollisions()->raycast(
            _engine->getRenderer()->getCameraSystem().getActiveSceneCamera(),
            _engine->getInput()->GetMousePosition(), windowWidth, windowHeight);

        dzemikk::MeshRenderer* currentRenderer = nullptr;

        if (collider) {
            currentRenderer = collider->getOwner()->getComponent<dzemikk::MeshRenderer>();
        }

        constexpr float hoverStrength = 0.5f;

        if (currentRenderer != lastHitRenderer) {

            if (lastHitRenderer && lastHitRenderer->isValid()) {

                glm::vec4 color = lastHitRenderer->getColor();

                color.r += hoverStrength;
                color.g += hoverStrength;
                color.b += hoverStrength;

                color = glm::clamp(color, 0.0f, 1.0f);

                lastHitRenderer->setColor(color);
            }

            if (currentRenderer && currentRenderer->isValid()) {

                glm::vec4 color = currentRenderer->getColor();
                color.r -= hoverStrength;
                color.g -= hoverStrength;
                color.b -= hoverStrength;

                color = glm::clamp(color, 0.0f, 1.0f);

                currentRenderer->setColor(color);
            }

            lastHitRenderer = currentRenderer;
        }
    });

    _engine->getInput()->OnMouseButtonPressed.addListener([this](dzemikk::MouseButtonPressedEvent& event) {
        if (event.GetMouseButton() != GLFW_MOUSE_BUTTON_LEFT) {
            return;
        }

        int windowWidth = 0;
        int windowHeight = 0;
        glfwGetWindowSize(_engine->getWindow()->nativeHandle(), &windowWidth, &windowHeight);

        dzemikk::Collider* collider = _engine->getCollisions()->raycast(
        _engine->getRenderer()->getCameraSystem().getActiveSceneCamera(),
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
    _playerEntity = playerGO->addComponent<game::PlayerEntity>();
    _playerMovement = playerGO->addComponent<game::PlayerMovement>();
    _playerMovement->setPlayerEntity(_playerEntity);
    _playerMovement->setSpeed(0.25F);

    _hexGrid = _worldGO->getComponent<game::World>()->getGrid();
    _playerMovement->setHexGrid(_hexGrid);
}
