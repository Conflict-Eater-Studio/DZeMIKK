#include "game.h"

#include "../../DZeMIKK/include/ecs/components/animator.h"
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
#include "ecs/components/ui/uiButtonActionRegistry.h"
#include "ecs/components/ui/uiCheckbox.h"
#include "ecs/components/ui/uiCheckboxActionRegistry.h"
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

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <queue>
#include <random>
#include <set>
struct Hex {
    int q, r;

    bool operator<(const Hex& other) const {
        return std::tie(q, r) < std::tie(other.q, other.r);
    }
};

glm::vec3 hexToWorld(int q, int r, float size) {
    float x = size * sqrt(3.0f) * (q + r * 0.5f);
    float z = size * 1.5f * r;
    return glm::vec3(x, 0.0f, z);
}

glm::vec3 hexToWorld(int q, int r, float size, float spacing = 0.1f, float maxHeight = 0.3f) {
    float width = sqrt(3.0f) * size + spacing;
    float verticalSpacing = 1.5f * size + spacing;

    float x = width * (q + r * 0.5f);
    float z = verticalSpacing * r;

    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<float> heightDist(0.0f, maxHeight);
    float y = heightDist(rng);

    return glm::vec3(x, y, z);
}

void createHexIsland(dzemikk::Scene& scene, dzemikk::Model* mesh, dzemikk::Material* materialA,
                     dzemikk::Material* materialB, int tileCount, float size, float spacing,
                     float maxHeight) {
    std::set<Hex> island;
    std::vector<Hex> frontier;

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);

    const std::vector<std::pair<int, int>> directions = {{1, 0},  {0, 1},  {-1, 1},
                                                         {-1, 0}, {0, -1}, {1, -1}};

    island.insert({0, 0});
    frontier.push_back({0, 0});

    while (island.size() < tileCount && !frontier.empty()) {
        int idx = rng() % frontier.size();
        Hex current = frontier[idx];

        for (auto& dir : directions) {
            Hex next = {current.q + dir.first, current.r + dir.second};

            if (island.contains(next))
                continue;

            if (chance(rng) < 0.6f) {
                island.insert(next);
                frontier.push_back(next);
            }
        }

        frontier.erase(frontier.begin() + idx);
    }

    for (const auto& hex : island) {
        if (chance(rng) < 0.1f)
            continue;

        glm::vec3 pos = hexToWorld(hex.q, hex.r, size, spacing, maxHeight);

        auto tile = scene.createGameObject();
        tile->transform()->setPosition(pos);
        tile->transform()->setScale(glm::vec3(1.0f));

        tile->transform()->setRotation(glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0)));

        auto renderer = tile->addComponent<dzemikk::MeshRenderer>();
        renderer->setModel(mesh);
        renderer->setTransform(tile->transform());

        if ((hex.q + hex.r) % 2 == 0)
            renderer->setMaterial(0, materialA);
        else
            renderer->setMaterial(0, materialB);

        auto collider = tile->addComponent<dzemikk::Collider>();
        collider->setModel(mesh);
        collider->setTransform(tile->transform());
    }
}

Game::Game(dzemikk::Engine* engine) : engine(engine) {}
void Game::start() {
    auto mainScenePtr = std::make_shared<dzemikk::Scene>();
    engine->getSceneManager()->loadScene(mainScenePtr);
    engine->getSceneManager()->setActiveScene(mainScenePtr);

    auto GO = mainScenePtr->createGameObject("Player");
    auto animator = GO->addComponent<dzemikk::Animator>();
    // auto m1 = engine->getAssetManager()->get<dzemikk::Model>("models/pole.fbx");
    // auto skybox = engine->getAssetManager()->get<dzemikk::Skybox>("textures/Daylight Box_Pieces");
    // skybox.get()->setShader(engine->getAssetManager()->get<dzemikk::Shader>("shaders/skybox").get());
    // engine->getRenderer()->setSkybox(skybox.get());
    //
    // auto mainScenePtr = std::make_shared<dzemikk::Scene>();
    // engine->getSceneManager()->loadScene(mainScenePtr);
    // engine->getSceneManager()->setActiveScene(mainScenePtr);
    //
    // auto cameraGO = mainScenePtr->createGameObject();
    // cameraGO->transform()->setPosition(glm::vec3(1.5f, 1.5f, 3.0f));
    // auto camera = cameraGO->addComponent<dzemikk::Camera>();
    // camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
    // engine->getRenderer()->setActiveSceneCamera(camera);
    //
    // auto shaderA = engine->getAssetManager()->get<dzemikk::Shader>("shaders/tile1");
    // auto materialA = new dzemikk::Material();
    // materialA->setShader(shaderA.get());
    //
    // auto shaderB = engine->getAssetManager()->get<dzemikk::Shader>("shaders/tile2");
    // auto materialB = new dzemikk::Material();
    // materialB->setShader(shaderB.get());
    //
    // auto tileMesh = engine->getAssetManager()->get<dzemikk::Model>("models/pole.fbx");
    //
    // createHexIsland(*mainScenePtr, tileMesh.get(), materialA, materialB, 50, 1.0f, 0.15f, 0.5f);
    //
    // auto playerGO = mainScenePtr->createGameObject();
    // playerGO->transform()->setPosition(glm::vec3(0.0f, 2.5f, 0.0f));
    //
    // auto playerMeshR = playerGO->addComponent<dzemikk::MeshRenderer>();
    // auto playerMesh = engine->getAssetManager()->getPrimitive(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Capsule);
    //
    // playerMeshR->setModel(new dzemikk::Model());
    // playerMeshR->getModel()->addMesh(std::shared_ptr<dzemikk::Mesh>(playerMesh), 0);
    // playerMeshR->setTransform(playerGO->transform());
    // playerMeshR->setMaterial(0, materialA);
    //
    // auto chestGO = mainScenePtr->createGameObject();
    // chestGO->transform()->setPosition(glm::vec3(-4.0f, 2.5f, 0.0f));
    // chestGO->transform()->setRotation(glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0)));
    // auto chestMeshR = chestGO->addComponent<dzemikk::MeshRenderer>();
    // auto chestMesh = engine->getAssetManager()->get<dzemikk::Model>("models/skrzynia.fbx");
    // chestMeshR->setModel(chestMesh.get());
    // chestMeshR->setTransform(chestGO->transform());
    // chestMeshR->setMaterial(0, materialA);
    //
    // auto enemyGO = mainScenePtr->createGameObject();
    // enemyGO->transform()->setPosition(glm::vec3(2.0f, 1.5f, 0.0f));
    // enemyGO->transform()->setScale(glm::vec3(.01f, .01f, 0.01f));
    // auto enemyMeshR = enemyGO->addComponent<dzemikk::MeshRenderer>();
    // auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/Body Block.fbx");
    // enemyMeshR->setModel(enemyMesh.get());
    // enemyMeshR->setTransform(enemyGO->transform());
    // enemyMeshR->setMaterial(0, materialA);
    // enemyMeshR->setMaterial(1, materialB);
    //
    // auto cameraUIGO = mainScenePtr->createGameObject();
    // cameraUIGO->transform()->setPosition(glm::vec3(0.0f, 0.0f, 1.0f));
    // auto cameraUI = cameraUIGO->addComponent<dzemikk::Camera>();
    //
    // cameraUI->setOrthographic(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f);
    // engine->getRenderer()->setActiveUICamera(cameraUI);
    //
    // auto quadMesh = engine->getAssetManager()->getPrimitive(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);
    //
    // auto quadShader = engine->getAssetManager()->get<dzemikk::Shader>("shaders/quad");
    // auto quadMaterial = new dzemikk::Material();
    // quadMaterial->setShader(quadShader.get());
    //
    // auto font = engine->getAssetManager()->get<dzemikk::Font>("fonts/UncialAntiqua-Regular.ttf");
    // auto* canvasGo = mainScenePtr->createGameObject("Canvas");
    // auto* canvas = canvasGo->addComponent<dzemikk::Canvas>();
    // (void)canvas;
    //
    // dzemikk::UIButtonActionRegistry::get().registerAction(
    //     "demo.button.click", [](dzemikk::UIButton& button) {
    //         return [&button]() {
    //             const auto* owner = button.getOwner();
    //             if (owner != nullptr) {
    //                 spdlog::info("Button click on '{}'", owner->getName());
    //             } else {
    //                 spdlog::info("Button click");
    //             }
    //         };
    //     });
    // dzemikk::UIButtonActionRegistry::get().registerAction(
    //     "demo.button.enter",
    //     [](dzemikk::UIButton&) { return []() { spdlog::info("Button hover"); }; });
    // dzemikk::UIButtonActionRegistry::get().registerAction(
    //     "demo.button.exit",
    //     [](dzemikk::UIButton&) { return []() { spdlog::info("Button exit"); }; });
    //
    // auto* canvasRect = canvasGo->rectTransform();
    // canvasRect->setSize({1920.0F, 1080.0F});
    //
    // auto* buttonGo = mainScenePtr->createGameObject("Button", canvasGo);
    // dzemikk::UIButton::build(*buttonGo, dzemikk::UIButtonParams{
    //                                         .onClickActionId = "demo.button.click",
    //                                         .onEnterActionId = "demo.button.enter",
    //                                         .onExitActionId = "demo.button.exit",
    //                                         .rectTransformParams =
    //                                             {
    //                                                 .size = {300.0F, 150.0F},
    //                                                 .pivot = {0.5F, 0.5F},
    //                                                 .anchorMin = {0.5F, 0.5F},
    //                                                 .anchorMax = {0.5F, 0.5F},
    //                                             },
    //                                         .mesh = quadMesh,
    //                                         .material = quadMaterial,
    //                                     });
    // auto* buttonText = mainScenePtr->createGameObject("ButtonText", buttonGo);
    // auto* buttonTextRect = buttonText->rectTransform();
    // buttonTextRect->setSize({0.0F, 0.0F});
    // buttonTextRect->setAnchorMin({0.0F, 0.0F});
    // buttonTextRect->setAnchorMax({1.0F, 1.0F});
    // buttonTextRect->setPosition({0.0F, 0.0F});
    // buttonTextRect->setPivot({0.5F, 0.5F});
    // auto* buttonTextRenderer = buttonText->addComponent<dzemikk::UITextRenderer>();
    // buttonTextRenderer->text = "Click Me!";
    // buttonTextRenderer->font = font.get();
    // buttonTextRenderer->scale = 1.0F;
    // buttonTextRenderer->color = glm::vec3(0.0F, 0.0F, 0.0F);
    // buttonTextRenderer->horizontalAlign = dzemikk::UITextRenderer::HorizontalAlign::Center;
    // buttonTextRenderer->verticalAlign = dzemikk::UITextRenderer::VerticalAlign::Middle;
    //
    // // --- slider
    // constexpr float kSliderWidth = 400.0F;
    // constexpr float kSliderTrackHeight = 20.0F;
    // constexpr float kSliderHandleSize = 40.0F;
    //
    // auto* sliderGo = mainScenePtr->createGameObject("Slider", canvasGo);
    // auto* uiSlider = sliderGo->addComponent<dzemikk::UISlider>();
    // auto* backgroundGo = mainScenePtr->createGameObject("Background", sliderGo);
    // auto* fillGo = mainScenePtr->createGameObject("Fill", backgroundGo);
    // auto* handleGo = mainScenePtr->createGameObject("Handle", backgroundGo);
    //
    // uiSlider->setBackgroundColor(dzemikk::Colors::White);
    // uiSlider->setHandleColor(dzemikk::Colors::White);
    // uiSlider->setHandleHoverColor(dzemikk::Colors::fromHex("#DDDDDD"));
    // uiSlider->setHandlePressedColor(dzemikk::Colors::fromHex("#BBBBBB"));
    // uiSlider->setFillColor(dzemikk::Colors::fromHex("#00BBFF"));
    //
    // sliderGo->rectTransform()->setPosition({0.0F, 300.0F});
    // sliderGo->rectTransform()->setAnchorMin({0.5F, 0.5F});
    // sliderGo->rectTransform()->setAnchorMax({0.5F, 0.5F});
    // sliderGo->rectTransform()->setPivot({0.5F, 0.5F});
    // sliderGo->rectTransform()->setSize({kSliderWidth, kSliderHandleSize});
    //
    // auto* bgRect = backgroundGo->rectTransform();
    // bgRect->setAnchorMin({0.5F, 0.5F});
    // bgRect->setAnchorMax({0.5F, 0.5F});
    // bgRect->setPivot({0.5F, 0.5F});
    // bgRect->setSize({kSliderWidth, kSliderTrackHeight});
    // bgRect->setZIndex(10);
    //
    // auto* fillRect = fillGo->rectTransform();
    // fillRect->setAnchorMin({0.0F, 0.5F});
    // fillRect->setAnchorMax({0.0F, 0.5F});
    // fillRect->setPivot({0.0F, 0.5F});
    // fillRect->setPosition({0.0F, 0.0F});
    // fillRect->setSize({0.0F, kSliderTrackHeight});
    // fillRect->setZIndex(11);
    //
    // auto* handleRect = handleGo->rectTransform();
    // handleRect->setAnchorMin({0.0F, 0.5F});
    // handleRect->setAnchorMax({0.0F, 0.5F});
    // handleRect->setPivot({0.5F, 0.5F});
    // handleRect->setPosition({0.0F, 0.0F});
    // handleRect->setSize({kSliderHandleSize, kSliderHandleSize});
    // handleRect->setZIndex(12);
    //
    // auto* bgSprite = backgroundGo->addComponent<dzemikk::ImageRenderer>();
    // bgSprite->setMesh(quadMesh);
    // bgSprite->setMaterial(quadMaterial);
    // bgSprite->setRectTransform(bgRect);
    //
    // auto* fillSprite = fillGo->addComponent<dzemikk::ImageRenderer>();
    // fillSprite->setMesh(quadMesh);
    // fillSprite->setMaterial(quadMaterial);
    // fillSprite->setRectTransform(fillRect);
    //
    // auto* handleSprite = handleGo->addComponent<dzemikk::ImageRenderer>();
    // handleSprite->setMesh(quadMesh);
    // handleSprite->setMaterial(quadMaterial);
    // handleSprite->setRectTransform(handleRect);
    //
    // uiSlider->setBackgroundSpriteRenderer(bgSprite);
    // uiSlider->setFillSpriteRenderer(fillSprite);
    // uiSlider->setHandleSpriteRenderer(handleSprite);
    //
    // uiSlider->setOnValueChanged([fillRect, handleRect](float value) {
    //     const float clamped = std::clamp(value, 0.0F, 1.0F);
    //     fillRect->setSize({kSliderWidth * clamped, kSliderTrackHeight});
    //     handleRect->setPosition({kSliderWidth * clamped, 0.0F});
    //     spdlog::info("Slider value: {:.2f}", clamped);
    // });
    //
    // uiSlider->onValueChanged(0.35F);
    //
    // // --- Checkbox
    // dzemikk::UICheckboxActionRegistry::get().registerAction(
    //     "demo.checkbox.click", [](dzemikk::UICheckbox& checkbox) {
    //         return [&checkbox]() {
    //             const auto* owner = checkbox.getOwner();
    //             if (owner != nullptr) {
    //                 spdlog::info("Checkbox click on '{}'", owner->getName());
    //             } else {
    //                 spdlog::info("Checkbox click");
    //             }
    //         };
    //     });
    // auto* checkboxGo = mainScenePtr->createGameObject("Checkbox", canvasGo);
    // auto* uiCheckbox = checkboxGo->addComponent<dzemikk::UICheckbox>();
    // auto* checkboxRect = checkboxGo->rectTransform();
    // checkboxRect->setPosition({0.0F, -300.0F});
    // checkboxRect->setAnchorMin({0.5F, 0.5F});
    // checkboxRect->setAnchorMax({0.5F, 0.5F});
    // checkboxRect->setPivot({0.5F, 0.5F});
    // checkboxRect->setSize({30.0F, 30.0F});
    // checkboxRect->setZIndex(0);
    // auto* checkboxSprite = checkboxGo->addComponent<dzemikk::ImageRenderer>();
    // checkboxSprite->setMesh(quadMesh);
    // checkboxSprite->setMaterial(quadMaterial);
    // checkboxSprite->setRectTransform(checkboxRect);
    // checkboxSprite->setColor(dzemikk::Colors::White);
    // uiCheckbox->setBackgroundSpriteRenderer(checkboxSprite);
    // auto* checkmarkGo = mainScenePtr->createGameObject("Checkmark", checkboxGo);
    // auto* checkmarkRect = checkmarkGo->rectTransform();
    // checkmarkRect->setAnchorMin({0.1F, 0.1F});
    // checkmarkRect->setAnchorMax({0.9F, 0.9F});
    // checkmarkRect->setPivot({0.5F, 0.5F});
    // checkmarkRect->setPosition({0.0F, 0.0F});
    // checkmarkRect->setSize({0.0F, 0.0F});
    // checkmarkRect->setZIndex(1);
    // auto* checkmarkSprite = checkmarkGo->addComponent<dzemikk::ImageRenderer>();
    // checkmarkSprite->setMesh(quadMesh);
    // checkmarkSprite->setMaterial(quadMaterial);
    // checkmarkSprite->setRectTransform(checkmarkRect);
    // checkmarkSprite->setColor(dzemikk::Colors::Red);
    // uiCheckbox->setCheckmarkSpriteRenderer(checkmarkSprite);
    // uiCheckbox->setOnClickActionId("demo.checkbox.click");
    //
    // FMOD::System* system;
    // FMOD::System_Create(&system);
    // system->init(512, FMOD_INIT_NORMAL, nullptr);
    //
    // engine->getAssetManager()->setFMODSystem(system);
    //
    // auto sound = engine->getAssetManager()->get<dzemikk::Sound>("audio/prime_wznoszeniePol.wav");
    // sound.get()->play(system);
    //
    // engine->SetUserUpdateCallback([this, playerGO]() {
    //     if (!engine || !engine->getInput()) {
    //         return;
    //     }
    //
    //     if (engine->getInput()->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
    //         int windowWidth, windowHeight;
    //         glfwGetWindowSize(engine->getWindow()->nativeHandle(), &windowWidth, &windowHeight);
    //         dzemikk::Collider* collider = engine->getCollisions()->raycast(engine->getRenderer()->getActiveSceneCamera(), engine->getInput()->GetMousePosition(), windowWidth, windowHeight);
    //
    //        if (collider && collider != nullptr) {
    //             auto hit = collider->getOwner()->getComponent<dzemikk::Transform>();
    //             collider->getOwner()->getComponent<dzemikk::MeshRenderer>()->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    //            glm::vec3 position = hit->getPosition();
    //             position.y += 1.5f;
    //             playerGO->transform()->setPosition(position);
    //        }
    //     }
    // });
}
void Game::setupScene() {
    mainScene = std::make_shared<dzemikk::Scene>();
    engine->getSceneManager()->loadScene(mainScene);
    engine->getSceneManager()->setActiveScene(mainScene);
}

void Game::setupSkybox() {
    auto skybox = engine->getAssetManager()->get<dzemikk::Skybox>("textures/Daylight Box_Pieces");
    skybox.get()->setShader(engine->getAssetManager()->get<dzemikk::Shader>("shaders/skybox").get());
    engine->getRenderer()->setSkybox(skybox.get());
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
    materialA->setShader(shaderA.get());

    auto shaderB = engine->getAssetManager()->get<dzemikk::Shader>("shaders/tile2");
    materialB = new dzemikk::Material();
    materialB->setShader(shaderB.get());

    auto quadShader = engine->getAssetManager()->get<dzemikk::Shader>("shaders/quad");
    quadMaterial = new dzemikk::Material();
    quadMaterial->setShader(quadShader.get());
}

void Game::setupWorld() {
    auto tileMesh = engine->getAssetManager()->get<dzemikk::Model>("models/pole.fbx");

    createHexIsland(*mainScene, tileMesh.get(), materialA, materialB, 50, 1.0f, 0.15f, 0.5f);

    setupPlayer();
    setupChest();
    setupEnemy();
}

void Game::setupPlayer() {
    playerGO = mainScene->createGameObject();
    playerGO->transform()->setPosition(glm::vec3(0.0f, 2.5f, 0.0f));

    auto playerMeshR = playerGO->addComponent<dzemikk::MeshRenderer>();
    auto playerMesh =
        engine->getAssetManager()->getPrimitive(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Capsule);

    playerMeshR->setModel(new dzemikk::Model());
    playerMeshR->getModel()->addMesh(std::shared_ptr<dzemikk::Mesh>(playerMesh), 0);
    playerMeshR->setTransform(playerGO->transform());
    playerMeshR->setMaterial(0, materialA);
}

void Game::setupChest() {
    auto chestGO = mainScene->createGameObject();
    chestGO->transform()->setPosition(glm::vec3(-4.0f, 2.5f, 0.0f));
    chestGO->transform()->setRotation(glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0)));

    auto chestMeshR = chestGO->addComponent<dzemikk::MeshRenderer>();
    auto chestMesh = engine->getAssetManager()->get<dzemikk::Model>("models/skrzynia.fbx");

    chestMeshR->setModel(chestMesh.get());
    chestMeshR->setTransform(chestGO->transform());
    chestMeshR->setMaterial(0, materialA);
}

void Game::setupEnemy() {
    auto enemyGO = mainScene->createGameObject();
    enemyGO->transform()->setPosition(glm::vec3(2.0f, 1.5f, 0.0f));
    enemyGO->transform()->setScale(glm::vec3(.01f, .01f, 0.01f));

    auto enemyMeshR = enemyGO->addComponent<dzemikk::MeshRenderer>();
    auto enemyMesh = engine->getAssetManager()->get<dzemikk::Model>("models/Body Block.fbx");

    enemyMeshR->setModel(enemyMesh.get());
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
    auto quadMesh = engine->getAssetManager()->getPrimitive(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);
    auto font = engine->getAssetManager()->get<dzemikk::Font>("fonts/UncialAntiqua-Regular.ttf");

    dzemikk::UIButtonActionRegistry::get().registerAction(
        "demo.button.click", [](dzemikk::UIButton& button) {
            return [&button]() {
                const auto* owner = button.getOwner();
                if (owner != nullptr) {
                    spdlog::info("Button click on '{}'", owner->getName());
                } else {
                    spdlog::info("Button click");
                }
            };
        });

    dzemikk::UIButtonActionRegistry::get().registerAction(
        "demo.button.enter",
        [](dzemikk::UIButton&) { return []() { spdlog::info("Button hover"); }; });

    dzemikk::UIButtonActionRegistry::get().registerAction(
        "demo.button.exit",
        [](dzemikk::UIButton&) { return []() { spdlog::info("Button exit"); }; });

    auto* buttonGo = mainScene->createGameObject("Button", canvasGo);

    dzemikk::UIButton::build(*buttonGo,
                             dzemikk::UIButtonParams{
                                 .onClickActionId = "demo.button.click",
                                 .onEnterActionId = "demo.button.enter",
                                 .onExitActionId = "demo.button.exit",
                                 .rectTransformParams =
                                     {
                                         .size = {300.0F, 150.0F},
                                         .pivot = {0.5F, 0.5F},
                                         .anchorMin = {0.5F, 0.5F},
                                         .anchorMax = {0.5F, 0.5F},
                                     },
                                 .mesh = quadMesh,
                                 .material = quadMaterial,
                             });

    auto* buttonText = mainScene->createGameObject("ButtonText", buttonGo);
    auto* buttonTextRect = buttonText->rectTransform();

    buttonTextRect->setSize({0.0F, 0.0F});
    buttonTextRect->setAnchorMin({0.0F, 0.0F});
    buttonTextRect->setAnchorMax({1.0F, 1.0F});
    buttonTextRect->setPosition({0.0F, 0.0F});
    buttonTextRect->setPivot({0.5F, 0.5F});

    auto* buttonTextRenderer = buttonText->addComponent<dzemikk::UITextRenderer>();
    buttonTextRenderer->text = "Click Me!";
    buttonTextRenderer->font = font.get();
    buttonTextRenderer->scale = 1.0F;
    buttonTextRenderer->color = glm::vec3(0.0F, 0.0F, 0.0F);
    buttonTextRenderer->horizontalAlign = dzemikk::UITextRenderer::HorizontalAlign::Center;
    buttonTextRenderer->verticalAlign = dzemikk::UITextRenderer::VerticalAlign::Middle;
}

void Game::setupSlider(dzemikk::GameObject* canvasGo) {
    constexpr float kSliderWidth = 400.0F;
    constexpr float kSliderTrackHeight = 20.0F;
    constexpr float kSliderHandleSize = 40.0F;

    auto quadMesh = engine->getAssetManager()->getPrimitive(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);

    auto* sliderGo = mainScene->createGameObject("Slider", canvasGo);
    auto* uiSlider = sliderGo->addComponent<dzemikk::UISlider>();

    auto* backgroundGo = mainScene->createGameObject("Background", sliderGo);
    auto* fillGo = mainScene->createGameObject("Fill", backgroundGo);
    auto* handleGo = mainScene->createGameObject("Handle", backgroundGo);

    uiSlider->setBackgroundColor(dzemikk::Colors::White);
    uiSlider->setHandleColor(dzemikk::Colors::White);
    uiSlider->setHandleHoverColor(dzemikk::Colors::fromHex("#DDDDDD"));
    uiSlider->setHandlePressedColor(dzemikk::Colors::fromHex("#BBBBBB"));
    uiSlider->setFillColor(dzemikk::Colors::fromHex("#00BBFF"));

    sliderGo->rectTransform()->setPosition({0.0F, 300.0F});
    sliderGo->rectTransform()->setAnchorMin({0.5F, 0.5F});
    sliderGo->rectTransform()->setAnchorMax({0.5F, 0.5F});
    sliderGo->rectTransform()->setPivot({0.5F, 0.5F});
    sliderGo->rectTransform()->setSize({kSliderWidth, kSliderHandleSize});

    auto* bgRect = backgroundGo->rectTransform();
    bgRect->setAnchorMin({0.5F, 0.5F});
    bgRect->setAnchorMax({0.5F, 0.5F});
    bgRect->setPivot({0.5F, 0.5F});
    bgRect->setSize({kSliderWidth, kSliderTrackHeight});
    bgRect->setZIndex(10);

    auto* fillRect = fillGo->rectTransform();
    fillRect->setAnchorMin({0.0F, 0.5F});
    fillRect->setAnchorMax({0.0F, 0.5F});
    fillRect->setPivot({0.0F, 0.5F});
    fillRect->setPosition({0.0F, 0.0F});
    fillRect->setSize({0.0F, kSliderTrackHeight});
    fillRect->setZIndex(11);

    auto* handleRect = handleGo->rectTransform();
    handleRect->setAnchorMin({0.0F, 0.5F});
    handleRect->setAnchorMax({0.0F, 0.5F});
    handleRect->setPivot({0.5F, 0.5F});
    handleRect->setPosition({0.0F, 0.0F});
    handleRect->setSize({kSliderHandleSize, kSliderHandleSize});
    handleRect->setZIndex(12);

    auto* bgSprite = backgroundGo->addComponent<dzemikk::ImageRenderer>();
    bgSprite->setMesh(quadMesh);
    bgSprite->setMaterial(quadMaterial);
    bgSprite->setRectTransform(bgRect);

    auto* fillSprite = fillGo->addComponent<dzemikk::ImageRenderer>();
    fillSprite->setMesh(quadMesh);
    fillSprite->setMaterial(quadMaterial);
    fillSprite->setRectTransform(fillRect);

    auto* handleSprite = handleGo->addComponent<dzemikk::ImageRenderer>();
    handleSprite->setMesh(quadMesh);
    handleSprite->setMaterial(quadMaterial);
    handleSprite->setRectTransform(handleRect);

    uiSlider->setBackgroundSpriteRenderer(bgSprite);
    uiSlider->setFillSpriteRenderer(fillSprite);
    uiSlider->setHandleSpriteRenderer(handleSprite);

    uiSlider->setOnValueChanged([fillRect, handleRect](float value) {
        const float clamped = std::clamp(value, 0.0F, 1.0F);
        fillRect->setSize({kSliderWidth * clamped, kSliderTrackHeight});
        handleRect->setPosition({kSliderWidth * clamped, 0.0F});
        spdlog::info("Slider value: {:.2f}", clamped);
    });

    uiSlider->onValueChanged(0.35F);
}

void Game::setupCheckbox(dzemikk::GameObject* canvasGo) {
    auto quadMesh = engine->getAssetManager()->getPrimitive(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);

    dzemikk::UICheckboxActionRegistry::get().registerAction(
        "demo.checkbox.click", [](dzemikk::UICheckbox& checkbox) {
            return [&checkbox]() {
                const auto* owner = checkbox.getOwner();
                if (owner != nullptr) {
                    spdlog::info("Checkbox click on '{}'", owner->getName());
                } else {
                    spdlog::info("Checkbox click");
                }
            };
        });

    auto* checkboxGo = mainScene->createGameObject("Checkbox", canvasGo);
    auto* uiCheckbox = checkboxGo->addComponent<dzemikk::UICheckbox>();

    auto* checkboxRect = checkboxGo->rectTransform();
    checkboxRect->setPosition({0.0F, -300.0F});
    checkboxRect->setAnchorMin({0.5F, 0.5F});
    checkboxRect->setAnchorMax({0.5F, 0.5F});
    checkboxRect->setPivot({0.5F, 0.5F});
    checkboxRect->setSize({30.0F, 30.0F});
    checkboxRect->setZIndex(0);

    auto* checkboxSprite = checkboxGo->addComponent<dzemikk::ImageRenderer>();
    checkboxSprite->setMesh(quadMesh);
    checkboxSprite->setMaterial(quadMaterial);
    checkboxSprite->setRectTransform(checkboxRect);
    checkboxSprite->setColor(dzemikk::Colors::White);

    uiCheckbox->setBackgroundSpriteRenderer(checkboxSprite);

    auto* checkmarkGo = mainScene->createGameObject("Checkmark", checkboxGo);
    auto* checkmarkRect = checkmarkGo->rectTransform();

    checkmarkRect->setAnchorMin({0.1F, 0.1F});
    checkmarkRect->setAnchorMax({0.9F, 0.9F});
    checkmarkRect->setPivot({0.5F, 0.5F});
    checkmarkRect->setPosition({0.0F, 0.0F});
    checkmarkRect->setSize({0.0F, 0.0F});
    checkmarkRect->setZIndex(1);

    auto* checkmarkSprite = checkmarkGo->addComponent<dzemikk::ImageRenderer>();
    checkmarkSprite->setMesh(quadMesh);
    checkmarkSprite->setMaterial(quadMaterial);
    checkmarkSprite->setRectTransform(checkmarkRect);
    checkmarkSprite->setColor(dzemikk::Colors::Red);

    uiCheckbox->setCheckmarkSpriteRenderer(checkmarkSprite);
    uiCheckbox->setOnClickActionId("demo.checkbox.click");
}

void Game::setupAudio() {
    FMOD::System* system;
    FMOD::System_Create(&system);
    system->init(512, FMOD_INIT_NORMAL, nullptr);

    engine->getAssetManager()->setFMODSystem(system);

    auto sound = engine->getAssetManager()->get<dzemikk::Sound>("audio/prime_wznoszeniePol.wav");
    sound.get()->play(system);
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