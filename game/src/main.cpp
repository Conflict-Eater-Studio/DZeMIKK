#include "animation/animationclip.h"
#include "animation/animationtrack.h"
#include "assetManager/assetmanager.h"
#include "assetManager/primitiveMeshLibrary.h"
#include "audio/audioManager.h"
#include "audio/sound.h"
#include "core/engine.h"
#include "ecs/components/camera.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/ui/canvas.h"
#include "ecs/components/ui/gridLayout.h"
#include "ecs/components/ui/horizontalLayout.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/ui/uiActionRegistry.h"
#include "ecs/components/ui/uiBuilder.h"
#include "ecs/components/ui/uiButton.h"
#include "ecs/components/ui/uiEvent.h"
#include "ecs/components/ui/verticalLayout.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/scenemanager.h"
#include "ecs/serialize/gameobjectSerializer.h"
#include "events/key_event.h"
#include "input/input.h"
#include "renderer/font.h"
#include "renderer/material.h"
#include "renderer/model.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "scripts/world/world.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <tuple>

using namespace game;

void handleButtonClick(const dzemikk::UIEvent& event) {
    spdlog::info("Clicked, sender: {}", event.sender->typeName());
}

void handleSliderValueChanged(const dzemikk::UIEvent& event) {
    auto* slider = dynamic_cast<dzemikk::UISlider*>(event.sender);
    if (slider) {
        spdlog::info("Slider value changed: {}", slider->getValue());
    }
}

int main() {
    auto engine = std::make_shared<dzemikk::Engine>();

    Perlin perlin(1);

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
    material->setShader(shader.get());
    auto material2 = std::make_shared<dzemikk::Material>();
    material2->setShader(shader2.get());

    auto model = assetManager->get<dzemikk::Model>("models/pole.fbx");

    auto* enemyMesh =
        assetManager->getPrimitive(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Capsule);
    auto* enemyModel = new dzemikk::Model();
    enemyModel->addMesh(std::shared_ptr<dzemikk::Mesh>(enemyMesh), 0);

    auto* resourceMesh =
        assetManager->getPrimitive(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Sphere);
    auto* resourceModel = new dzemikk::Model();
    resourceModel->addMesh(std::shared_ptr<dzemikk::Mesh>(resourceMesh), 0);

    auto* rootGO = scene->createGameObject("Root");
    auto* worldGO = scene->createGameObject("World", rootGO);
    auto* world = worldGO->addComponent<World>(
        1, std::vector<std::tuple<int, int, std::vector<HexCoord::Direction>>>{
               {4, 6, {}},
               {6, 8, {}},
               {8, 10, {}},
               {10, 12, {HexCoord::Direction::R60}},
               {12, 14, {HexCoord::Direction::R300}},
               {14, 16, {}},
               {16, 18, {HexCoord::Direction::R60, HexCoord::Direction::R300}},
           });
    world->setModel(model.get());
    world->setMaterial(material.get());
    world->setMaterial2(material2.get());
    world->setEnemyModel(enemyModel);
    world->setResourceModel(resourceModel);

    auto* uiRootGO = scene->createGameObject("UI Root");
    auto* canvas = uiRootGO->addComponent<dzemikk::Canvas>();
    uiRootGO->rectTransform()->setSize({1920.0F, 1080.0F});
    auto* quadMesh = assetManager->getPrimitive(dzemikk::PrimitiveMeshLibrary::PrimitiveMesh::Quad);
    auto* quadShader = assetManager->get<dzemikk::Shader>("shaders/quad").get();
    auto quadMat = std::make_shared<dzemikk::Material>();
    quadMat->setShader(quadShader);

    auto* font = assetManager->get<dzemikk::Font>("fonts/UncialAntiqua-Regular.ttf").get();

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
                                                            .material = quadMat.get()};
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
                                                            .material = quadMat.get()};
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
                                                            .material = quadMat.get()};
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
    horiGO->enabled(true);
    vertGO->enabled(false);
    gridGO->enabled(false);

    // auto* btGO = dzemikk::UIBuilder::createButton(uiRootGO, {.name = "Test Button",
    //                                                           .position = {0.0F, 0.0F},
    //                                                           .size = {0.0F, 0.0F},
    //                                                           .anchorMin = {0.3F, 0.3F},
    //                                                           .anchorMax = {0.7F, 0.7F},
    //                                                           .text = "Click Me",
    //                                                           .textFont = font,
    //                                                           .mesh = quadMesh,
    //                                                           .material = quadMat.get()});
    // dzemikk::UIActionRegistry::get().registerAction(handleButtonClick, "btn.test");
    // auto* btn = btnGO->getComponent<dzemikk::UIButton>();
    // btn->addEventListener(dzemikk::UIEventType::Click, "btn.test");
    //
    // auto* sliderGO = dzemikk::UIBuilder::createSlider(uiRootGO, {.name = "Test Slider",
    //                                                              .position = {0.0F, 0.0F},
    //                                                              .size = {400.0F, 20.0F},
    //                                                              .anchorMin = {0.3F, 0.2F},
    //                                                              .anchorMax = {0.7F, 0.2F},
    //                                                              .bgMesh = quadMesh,
    //                                                              .fillMesh = quadMesh,
    //                                                              .handleMesh = quadMesh,
    //                                                              .bgMat = quadMat.get(),
    //                                                              .fillMat = quadMat.get(),
    //                                                              .handleMat = quadMat.get()});
    // dzemikk::UIActionRegistry::get().registerAction(handleSliderValueChanged, "slider.test");
    // auto* slider = sliderGO->getComponent<dzemikk::UISlider>();
    // slider->addEventListener(dzemikk::UIEventType::ValueChanged, "slider.test");
    // slider->setMaxValue(100.0F);
    // slider->setStep(0.1F);
    //
    // auto* checkboxGO = dzemikk::UIBuilder::createCheckbox(
    //     uiRootGO, {.name = "Test Checkbox",
    //                .position = {(1920.0F / 2.0F) - 25.0F, 1080.0F - 50.0F},
    //                .size = {50.0F, 50.0F},
    //                .bgMesh = quadMesh,
    //                .checkmarkMesh = quadMesh,
    //                .bgMat = quadMat.get(),
    //                .checkmarkMat = quadMat.get()});
    //
    // auto* dropdownGO = dzemikk::UIBuilder::createDropdown(
    //     uiRootGO, {
    //                   .name = "Test Dropdown",
    //                   .position = {(1920.0F / 4.0F) - 100.0F, 1080.0F - 150.0F},
    //                   .size = {400.0F, 70.0F},
    //                   .options =
    //                       {
    //                           {.text = "Option 1", .value = "opt1"},
    //                           {.text = "Option 2", .value = "opt2"},
    //                           {.text = "Option 3", .value = "opt3"},
    //                           {.text = "Option 4", .value = "opt4"},
    //                           {.text = "Option 5", .value = "opt5"},
    //                           {.text = "Option 6", .value = "opt6"},
    //                       },
    //                   .optionHeight = 70.0F,
    //                   .maxVisibleOptions = 3,
    //                   .textFont = font,
    //                   .bgMesh = quadMesh,
    //                   .arrowMesh = quadMesh,
    //                   .optionMesh = quadMesh,
    //                   .optionsBgMesh = quadMesh,
    //                   .bgMat = quadMat.get(),
    //                   .arrowMat = quadMat.get(),
    //                   .optionMat = quadMat.get(),
    //                   .optionsBgMat = quadMat.get(),
    //               });
    //
    // auto* dropdown = dropdownGO->getComponent<dzemikk::UIDropdown>();

    // --- Sound test ---
    auto sound = assetManager->get<dzemikk::Sound>("audio/prime_wznoszeniePol.wav");
    auto* audio = engine->getAudioManager();
    engine->getInput()->OnKeyPressed.addListener([&audio, &sound](dzemikk::KeyPressedEvent& event) {
        if (event.GetKeyCode() == GLFW_KEY_SPACE) {
            spdlog::info("Playing sound");
            audio->play(*sound.get(), dzemikk::AudioManager::SoundType::SFX);
        }
    });
    // --- To save your ears :)
    audio->getMasterGroup()->setVolume(0.1F);

    engine->start();

    return 0;
}
