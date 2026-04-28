#include "animation/animationclip.h"
#include "animation/animationtrack.h"
#include "assetManager/assetmanager.h"
#include "assetManager/primitiveMeshLibrary.h"
#include "core/engine.h"
#include "ecs/components/camera.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/ui/imageRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/scenemanager.h"
#include "renderer/material.h"
#include "renderer/model.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "scripts/world/world.h"

#include <GLFW/glfw3.h>
#include <memory>
#include <tuple>

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

    engine->start();

    return 0;
}
