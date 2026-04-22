#include "animation/animationclip.h"
#include "animation/animationmodule.h"
#include "animation/animationstate.h"
#include "animation/animationstatemachine.h"
#include "animation/animationtrack.h"
#include "animation/quaterniontrack.h"
#include "animation/vectortrack.h"
#include "assetManager/assetmanager.h"
#include "audio/sound.h"
#include "core/engine.h"
#include "core/time.h"
#include "ecs/components/animator.h"
#include "ecs/components/camera.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/textRenderer.h"
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
#include "events/key_event.h"
#include "events/mouse_event.h"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "input/input.h"
#include "map/ChunkSpawner.h"
#include "map/HexCoord.h"
#include "map/grid.h"
#include "map/hexChunk.h"
#include "renderer/font.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/model.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "utils/perlin.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <array>
#include <format>
#include <iostream>
#include <memory>
#include <queue>
#include <random>
#include <set>
#include <stdexcept>
#include <string_view>

using namespace game;

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
    auto material = std::make_shared<dzemikk::Material>();
    material->setShader(shader.get());
    auto model = assetManager->get<dzemikk::Model>("models/pole.fbx");

    auto* rootGO = scene->createGameObject("Root");
    auto* mapGO = scene->createGameObject("Map", rootGO);

    Grid grid;
    grid.makeChunk({0, 0}, {.steps = 6});
    std::size_t idx = grid.makeChunk(0, HexCoord::Direction::R0, {.steps = 6}).value_or(-1);
    engine->getInput()->OnKeyPressed.addListener([&](const dzemikk::KeyPressedEvent& event) {
        if (event.GetKeyCode() == GLFW_KEY_SPACE) {
            auto rng = std::mt19937(std::random_device{}());
            auto dist = std::uniform_int_distribution<int>(4, 20);
            auto dirDist = std::uniform_int_distribution<int>(0, 2);
            std::vector<HexCoord::Direction> dirs = {
                HexCoord::Direction::R0, HexCoord::Direction::R60, HexCoord::Direction::R300};
            std::size_t newIdx =
                grid.makeChunk(idx, dirs.at(dirDist(rng)), {.steps = dist(rng)}).value_or(-1);
            spdlog::info("New chunk index: {}", newIdx);
            if (newIdx != -1) {
                idx = newIdx;
                auto newHexes = grid.getChunks().at(idx).getHexes();
                for (auto cell : newHexes) {
                    auto* obj = scene->createGameObject("Hex", mapGO);
                    cell.setHeight(perlin.noise(static_cast<float>(cell.q()) * 0.1F,
                                                static_cast<float>(cell.r()) * 0.1F) *
                                   2.0F);
                    obj->transform()->setPosition(
                        cell.toWorldPosition(std::numbers::sqrt3_v<float> / 2.0F, 0.0F));
                    obj->transform()->setScale({1.0F, 1.0F, 1.0F});
                    obj->transform()->setRotation(
                        glm::angleAxis(glm::radians(-90.0F), glm::vec3{1.0F, 0.0F, 0.0F}));
                    auto* mesh = obj->addComponent<dzemikk::MeshRenderer>();
                    mesh->setModel(model.get());
                    mesh->setMaterial(0, material.get());
                    mesh->setTransform(obj->transform());
                }
            }
        }
    });

    for (auto cell : grid.getHexes()) {
        auto* obj = scene->createGameObject("Hex", mapGO);
        cell.coord.setHeight(perlin.noise(static_cast<float>(cell.coord.q()) * 0.1F,
                                          static_cast<float>(cell.coord.r()) * 0.1F) *
                             2.0F);
        obj->transform()->setPosition(
            cell.coord.toWorldPosition(std::numbers::sqrt3_v<float> / 2.0F, 0.0F));
        obj->transform()->setScale({1.0F, 1.0F, 1.0F});
        obj->transform()->setRotation(
            glm::angleAxis(glm::radians(-90.0F), glm::vec3{1.0F, 0.0F, 0.0F}));
        auto* mesh = obj->addComponent<dzemikk::MeshRenderer>();
        mesh->setModel(model.get());
        mesh->setMaterial(0, material.get());
        mesh->setTransform(obj->transform());
    }

    engine->start();

    return 0;
}
