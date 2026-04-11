#include "core/engine.h"
#include "ecs/components/camera.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/textRenderer.h"
#include "ecs/components/monobehaviour.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "renderer/material.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "renderer/font.h"
#include "renderer/texture.h"
#include <ecs/scenemanager.h>
#include "assetManager/assetmanager.h"
#include "audio/sound.h"

#include <filesystem>
#include <iostream>
#include <queue>
#include <random>
#include <set>
#include <GLFW/glfw3.h>
#include <memory>

class TextUpdater : public dzemikk::MonoBehaviour {
  public:
    using Base = MonoBehaviour;

    dzemikk::TextRenderer* text = nullptr;
    float time = 0.0f;

    void update(double deltaTime) override {
        time += deltaTime;
        text->text = "Time: " + std::to_string((int)time);
    }

    [[nodiscard]] std::string typeName() const override {
        return "TextUpdater";
    };
};

class SpriteUpdater: public dzemikk::MonoBehaviour {
  public:
      using Base = MonoBehaviour;

      dzemikk::Transform* transform = nullptr;
      float time = 0.0f;

      void update(double deltaTime) override {
          time += deltaTime;

          float scaleX = 0.5f + 0.5f * sin(time);
          float scaleY = 1.0f;

          transform->setScale(glm::vec3(scaleX, scaleY, 1.0f));
      }

    [[nodiscard]] std::string typeName() const override {
          return "SpriteUpdater";
      };

};

void createHexIsland(dzemikk::Scene& scene, dzemikk::Mesh* mesh, dzemikk::Material* materialA,
                     dzemikk::Material* materialB, int tileCount, float size, float spacing = 0.1f,
                     float maxHeight = 0.3f); 

int main() {
    auto engine = std::make_shared<dzemikk::Engine>();

    auto skybox = engine->getAssetManager()->Get<dzemikk::Skybox>("textures/Daylight Box_Pieces");
    engine->getRenderer()->setSkybox(std::unique_ptr<dzemikk::Skybox>(skybox));

    auto font = engine->getAssetManager()->Get<dzemikk::Font>("fonts/UncialAntiqua-Regular.ttf");

    auto mainScenePtr = std::make_shared<dzemikk::Scene>();
    engine->getSceneManager()->loadScene(mainScenePtr);
    engine->getSceneManager()->setActiveScene(mainScenePtr);

    // --- Scene Camera
    auto cameraGO = mainScenePtr->createGameObject();
    cameraGO->transform()->setPosition(glm::vec3(1.5f, 1.5f, 3.0f));
    auto camera = cameraGO->addComponent<dzemikk::Camera>();
    camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
    engine->getRenderer()->setActiveSceneCamera(camera);

    // --- Tiles
    auto shaderA = engine->getAssetManager()->Get<dzemikk::Shader>("shaders/tile1");
    auto materialA = new dzemikk::Material();
    materialA->setShader(shaderA);

    auto shaderB = engine->getAssetManager()->Get<dzemikk::Shader>("shaders/tile2");
    auto materialB = new dzemikk::Material();
    materialB->setShader(shaderB);

    auto tileMesh = engine->getAssetManager()->Get<dzemikk::Mesh>("models/pole.fbx");
    
    createHexIsland(*mainScenePtr, tileMesh, materialA, materialB, 100000, 1.0f, 0.15f, 0.5f);

    // --- Player
    auto playerGO = mainScenePtr->createGameObject();
    playerGO->transform()->setPosition(glm::vec3(0.0f, 2.5f, 0.0f));
    auto playerMeshR = playerGO->addComponent<dzemikk::MeshRenderer>();
    auto playerMesh = engine->getAssetManager()->GetPrimitive(dzemikk::AssetManager::PrimitiveMesh::Capsule);
    playerMeshR->setMesh(playerMesh);
    playerMeshR->setTransform(playerGO->transform());
    playerMeshR->setMaterial(materialA);

    // UI Camera
    auto cameraUIGO = mainScenePtr->createGameObject();
    cameraUIGO->transform()->setPosition(glm::vec3(0.0f, 0.0f, 1.0f));
    auto cameraUI = cameraUIGO->addComponent<dzemikk::Camera>();

    cameraUI->setOrthographic(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f);
    engine->getRenderer()->setActiveUICamera(cameraUI);

    // --- Quad GameObject
    auto quadGO = new dzemikk::GameObject();
    quadGO->transform()->setPosition(glm::vec3(100.0f, 300.0f, 0.0f));
    quadGO->transform()->setScale(glm::vec3(100.0f, 100.0f, 1.0f)); 
    quadGO->transform()->setRotation(glm::quat());

    auto quadMesh =
        engine->getAssetManager()->GetPrimitive(dzemikk::AssetManager::PrimitiveMesh::Quad);

    auto quadShader = engine->getAssetManager()->Get<dzemikk::Shader>("shaders/quad");
    auto quadMaterial = new dzemikk::Material();
    quadMaterial->setShader(quadShader);

    auto quadRenderer = quadGO->addComponent<dzemikk::SpriteRenderer>();
    quadRenderer->setMesh(quadMesh);
    quadRenderer->setMaterial(quadMaterial);
    quadRenderer->setTransform(quadGO->transform());
    quadRenderer->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));

    auto tex = engine->getAssetManager()->Get<dzemikk::Texture>("textures/tex3.png");

    quadRenderer->setTexture(tex);

    auto quadGO2 = mainScenePtr->createGameObject();
    quadGO2->transform()->setPosition(glm::vec3(1500.0f, 950.0f, 0.0f));
    quadGO2->transform()->setScale(glm::vec3(400.0f, 50.0f, 1.0f));
    quadGO2->transform()->setRotation(glm::quat());

    auto quadRenderer2 = quadGO2->addComponent<dzemikk::SpriteRenderer>();
    quadRenderer2->setMesh(quadMesh);
    quadRenderer2->setMaterial(quadMaterial);
    quadRenderer2->setTransform(quadGO2->transform());
    quadRenderer2->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));

    auto quadGO3 = mainScenePtr->createGameObject();
    quadGO3->transform()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    quadGO3->transform()->setScale(glm::vec3(0.9f, 1.0f, 1.0f));
    quadGO3->transform()->setRotation(glm::quat());
    quadGO2->addChild(quadGO3);

    quadGO3->setName("QuadGO3");

    auto quadRenderer3 = quadGO3->addComponent<dzemikk::SpriteRenderer>();
    quadRenderer3->setMesh(quadMesh);
    quadRenderer3->setMaterial(quadMaterial);
    quadRenderer3->setTransform(quadGO3->transform());
    quadRenderer3->setColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

    auto quadSpriteUpdater = quadGO3->addComponent<SpriteUpdater>();
    quadSpriteUpdater->transform = quadGO3->transform();

    auto textGO = mainScenePtr->createGameObject();
    textGO->transform()->setPosition(glm::vec3(50.0f, 540.0f, 0.0f));

    auto text = textGO->addComponent<dzemikk::TextRenderer>();
    text->text = "Hello World!";
    text->font = font;
    text->scale = 1.0f;
    text->color = glm::vec3(1.0f, 1.0f, 1.0f);

    auto updater = textGO->addComponent<TextUpdater>();
    updater->text = text;

    // FOR TEST ONLY - DELETE THIS
    FMOD::System* system;
    FMOD::System_Create(&system);
    system->init(512, FMOD_INIT_NORMAL, nullptr);

    engine->getAssetManager()->system = system;

    auto sound = engine->getAssetManager()->Get<dzemikk::Sound>("audio/prime_coToZaHex.wav");
    sound->play(system);

    engine->start();

    return 0;
}

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

void createHexIsland(dzemikk::Scene& scene, dzemikk::Mesh* mesh, dzemikk::Material* materialA,
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
        renderer->setMesh(mesh);
        renderer->setTransform(tile->transform());

        if ((hex.q + hex.r) % 2 == 0)
            renderer->setMaterial(materialA);
        else
            renderer->setMaterial(materialB);
    }
}