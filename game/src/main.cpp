#include "animation/animationclip.h"
#include "animation/animationcurve.h"
#include "animation/animationstate.h"
#include "animation/animationstatemachine.h"
#include "animation/animationtrack.h"
#include "core/engine.h"
#include "ecs/components/animator.h"
#include "ecs/components/camera.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/monobehaviour.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/textRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "renderer/font.h"
#include "renderer/material.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "spdlog/fmt/bundled/std.h"
#include "spdlog/spdlog.h"

#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>
#include <glad/glad.h>
#include <iostream>
#include <memory>
#include <stb/stb_image.h>

class TextUpdater : public dzemikk::MonoBehaviour {
  public:
    using Base = MonoBehaviour;

    dzemikk::TextRenderer* text = nullptr;
    float time = 0.0f;

    void update(double deltaTime) override {
        time += deltaTime;
        text->text = "Time: " + std::to_string((int)time);
    }
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

};

dzemikk::Mesh* createCubeMesh();
dzemikk::Mesh* createQuadMesh();
void createCubeBoard(std::shared_ptr<dzemikk::Engine> engine, dzemikk::Scene& scene,
                     dzemikk::Mesh* cubeMesh, dzemikk::Material* materialA,
                     dzemikk::Material* materialB, int rows, int cols, float spacing);

dzemikk::Mesh* loadMeshFromFile(const std::string& path);
GLuint loadTextureFromFile(const std::string& path, bool flipVertical = true);

void createHexIsland(dzemikk::Scene& scene, dzemikk::Mesh* mesh, dzemikk::Material* materialA,
                     dzemikk::Material* materialB, int tileCount, float size, float spacing = 0.1f,
                     float maxHeight = 0.3f); 
void printAnimationInfo(const aiScene* scene) {
    spdlog::info("Animations: {}", scene->mNumAnimations);

    for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
        aiAnimation* anim = scene->mAnimations[i];

        spdlog::info("Animation {}:", i);
        spdlog::info("  Duration: {}", anim->mDuration);
        spdlog::info("  TicksPerSecond: {}", anim->mTicksPerSecond);
        spdlog::info("  Channels: {}", anim->mNumChannels);

        for (unsigned int j = 0; j < anim->mNumChannels; j++) {
            aiNodeAnim* channel = anim->mChannels[j];

            spdlog::info("    Channel {}: {}", j, channel->mNodeName.C_Str());
            spdlog::info("      Position keys: {}", channel->mNumPositionKeys);
            spdlog::info("      Rotation keys: {}", channel->mNumRotationKeys);
            spdlog::info("      Scaling keys: {}", channel->mNumScalingKeys);
        }
    }
}
int main() {
    auto engine = std::make_shared<dzemikk::Engine>();

    auto customSkybox = std::make_unique<dzemikk::Skybox>();
    customSkybox->loadCubemap({"./res/textures/Daylight Box_Pieces/right.png",
                               "./res/textures/Daylight Box_Pieces/left.png",
                               "./res/textures/Daylight Box_Pieces/top.png",
                               "./res/textures/Daylight Box_Pieces/bottom.png",
                               "./res/textures/Daylight Box_Pieces/front.png",
                               "./res/textures/Daylight Box_Pieces/back.png"});

    engine->getRenderer()->setSkybox(std::move(customSkybox));

    auto font = new dzemikk::Font();
    if (!font->load("./res/fonts/UncialAntiqua-Regular.ttf")) {
        std::cout << "Failed to load font\n";
    }

    dzemikk::Scene mainScene;

    engine->scene = &mainScene;

    // --- Scene Camera
    auto cameraGO = mainScene.createGameObject();
    cameraGO->transform()->setPosition(glm::vec3(1.5f, 1.5f, 3.0f));
    auto camera = cameraGO->addComponent<dzemikk::Camera>();
    camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    // Rejestracja kamery w rendererze
    engine->getRenderer()->setActiveSceneCamera(camera);

    // --- Cube GameObject
    const char* vertexSrc3D = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;

    // Atrybut instancji: model mat4 (4 wektory)
    layout(location = 2) in vec4 aModelRow0;
    layout(location = 3) in vec4 aModelRow1;
    layout(location = 4) in vec4 aModelRow2;
    layout(location = 5) in vec4 aModelRow3;

    layout (std140) uniform Matrices
    {
        mat4 projection;
        mat4 view;
    };

    out vec3 FragPos;
    out vec3 Normal;

    void main() {
        mat4 model = mat4(aModelRow0, aModelRow1, aModelRow2, aModelRow3);
        FragPos = vec3(model * vec4(aPos,1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * vec4(FragPos,1.0);
    }
    )";

    const char* fragmentSrc3D = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;

    uniform vec3 lightDir;      // Kierunek �wiat�a (np. glm::normalize(glm::vec3(-1.0, -1.0, -1.0)))
    uniform vec3 lightColor;    // Kolor �wiat�a (np. vec3(1.0,1.0,1.0))
    uniform vec3 objectColor;   // Kolor kostki (np. vec3(1.0,0.5,0.2))

    void main() {
        // Lambert: max(dot(N,L),0)
        vec3 norm = normalize(Normal);
        vec3 lightDirNorm = normalize(-lightDir); // je�li �wiat�o "idzie w kierunku"
        float diff = max(dot(norm, lightDirNorm), 0.0);

        vec3 diffuse = diff * lightColor;

        vec3 result = diffuse * objectColor;
        FragColor = vec4(result, 1.0);
    }
    )";

    auto shaderA = new dzemikk::Shader(vertexSrc3D, fragmentSrc3D);
    auto materialA = new dzemikk::Material();
    materialA->setShader(shaderA);

    const char* fragmentSrc3D_B = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;

    uniform vec3 lightDir;
    uniform vec3 lightColor;

    void main() {
        vec3 norm = normalize(Normal);
        vec3 lightDirNorm = normalize(-lightDir);
        float diff = max(dot(norm, lightDirNorm), 0.0);
        vec3 diffuse = diff * lightColor;
        FragColor = vec4(diffuse * vec3(0.2,0.5,1.0), 1.0); // niebieska kostka
    }
    )";
    auto shaderB = new dzemikk::Shader(vertexSrc3D, fragmentSrc3D_B);
    auto materialB = new dzemikk::Material();
    materialB->setShader(shaderB);

    auto cubeMesh = createCubeMesh();

    auto tileMesh = loadMeshFromFile("./res/models/pole.fbx");

    //createHexIsland(mainScene, tileMesh, materialA, materialB, 100000, 1.0f, 0.15f, 0.5f);

    // UI Camera
    auto cameraUIGO = mainScene.createGameObject();
    cameraUIGO->transform()->setPosition(glm::vec3(0.0f, 0.0f, 1.0f));

    auto cameraUI = cameraUIGO->addComponent<dzemikk::Camera>();

    cameraUI->setOrthographic(0.0f, 1920.0f,
                            0.0f, 1080.0f,
                            -1.0f, 1.0f
    );
    engine->getRenderer()->setActiveUICamera(cameraUI);

    // --- Quad GameObject
    auto quadGO = new dzemikk::GameObject();
    quadGO->transform()->setPosition(glm::vec3(100.0f, 300.0f, 0.0f));
    quadGO->transform()->setScale(glm::vec3(100.0f, 100.0f, 1.0f));
    quadGO->transform()->setRotation(glm::quat());

    auto quadMesh = createQuadMesh();

    const char* vertexSrcUI = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec2 aTexCoords;

    out vec2 TexCoords;

    uniform mat4 model;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * model * vec4(aPos, 1.0);
        TexCoords = aTexCoords;
    }
    )";

    const char* fragmentSrcUI = R"(
    #version 330 core
    in vec2 TexCoords;
    out vec4 FragColor;

    uniform sampler2D spriteTexture;
    uniform vec4 spriteColor; // RGBA

    void main()
    {
        vec4 texColor = texture(spriteTexture, TexCoords);
        FragColor = vec4(spriteColor.rgb, spriteColor.a * texColor.a);
    }
    )";

    auto quadShader = new dzemikk::Shader(vertexSrcUI, fragmentSrcUI);
    auto quadMaterial = new dzemikk::Material();
    quadMaterial->setShader(quadShader);

    auto quadRenderer = quadGO->addComponent<dzemikk::SpriteRenderer>();
    quadRenderer->setMesh(quadMesh);
    quadRenderer->setMaterial(quadMaterial);
    quadRenderer->setTransform(quadGO->transform());
    quadRenderer->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));

    auto quadGO2 = mainScene.createGameObject();
    quadGO2->transform()->setPosition(glm::vec3(1500.0f, 950.0f, 0.0f));
    quadGO2->transform()->setScale(glm::vec3(400.0f, 50.0f, 1.0f));
    quadGO2->transform()->setRotation(glm::quat());

    auto quadRenderer2 = quadGO2->addComponent<dzemikk::SpriteRenderer>();
    quadRenderer2->setMesh(quadMesh);
    quadRenderer2->setMaterial(quadMaterial);
    quadRenderer2->setTransform(quadGO2->transform());
    quadRenderer2->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));

    auto quadGO3 = mainScene.createGameObject();
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

    auto textGO = mainScene.createGameObject();
    textGO->transform()->setPosition(glm::vec3(50.0f, 540.0f, 0.0f));

    auto text = textGO->addComponent<dzemikk::TextRenderer>();
    text->text = "Hello World!";
    text->font = font;
    text->scale = 1.0f;
    text->color = glm::vec3(1.0f, 1.0f, 1.0f);

    auto updater = textGO->addComponent<TextUpdater>();
    updater->text = text;



    auto playerGO = mainScene.createGameObject();
    playerGO->transform()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    playerGO->transform()->setRotation(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)));
    playerGO->transform()->setScale(glm::vec3(10.0f));

    auto playerMesh = createCubeMesh();
    auto renderer = playerGO->addComponent<dzemikk::MeshRenderer>();
    renderer->setMesh(playerMesh);
    renderer->setMaterial(materialA);
    renderer->setTransform(playerGO->transform());

    playerGO->transform()->setScale(glm::vec3(1.f));
    playerGO->transform()->setRotation(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)));

    dzemikk::Animator* animator =  playerGO->addComponent<dzemikk::Animator>();

    std::shared_ptr<dzemikk::AnimationStateMachine> animationStateMachine = std::make_shared<dzemikk::AnimationStateMachine>();
    dzemikk::AnimationState* idleState = animationStateMachine->addState();

    dzemikk::AnimationState* moveState = animationStateMachine->addState("Move");

    dzemikk::AnimationClip* animationClip = new dzemikk::AnimationClip(2, 1);
    dzemikk::AnimationClip* animationClip2 = new dzemikk::AnimationClip(2, 1);

    dzemikk::AnimationTrack* animationTrack =  animationClip->addTrack();
    dzemikk::AnimationTrack* animationTrack2 = animationClip2->addTrack();

    idleState->setClip(animationClip);
    moveState->setClip(animationClip2);

    dzemikk::Transform* t = playerGO->transform();
    dzemikk::FloatProperty prop(
        [t]{ return t->getPosition().x; },          // getter
        [t](float v) {                                // setter
            auto pos = t->getPosition();              // copy
            pos.x = v;
            t->setPosition(pos);                      // write back
        });

    dzemikk::FloatProperty prop2(
        [t] {return t->getRotation().x; },
        [t](float eulerAngles) {
            auto rot = t->getRotation();
            auto newRot = glm::quat(glm::vec3(eulerAngles, 0.0f, 0.0f));

            newRot.y = rot.y;
            newRot.z = rot.z;

            t->setRotation(newRot);
        });
    animationTrack2->setProperty(prop);
    animationTrack2->addKey({0.0f, 0.0f});
    animationTrack2->addKey({0.5f, 0.5f});
    animationTrack2->addKey({1.0f, 1.0f});

    animationTrack->setProperty(prop2);
    animationTrack->addKey({0.0f, 0.0f});
    animationTrack->addKey({0.5f, 0.5f});
    animationTrack->addKey({1.0f, 1.0f});

    animator->setStateMachine(animationStateMachine);
    animator->play("Move");

    // Assimp::Importer importer;
    // const aiScene* scene = importer.ReadFile("./res/models/model.fbx", aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);
    // if (!scene || !scene->mRootNode) {
    //     spdlog::error("Failed to load model: {}", importer.GetErrorString());
    // }
    // spdlog::info("\n=== Animation Info ===");
    // printAnimationInfo(scene);
    engine->start();
    return 0;
}

dzemikk::Mesh* createCubeMesh() {
    dzemikk::Mesh* mesh = new dzemikk::Mesh();

    float vertices[] = {// --- Front face
                        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
                        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

                        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
                        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

                        // --- Back face
                        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.5f, 0.5f, -0.5f, 0.0f, 0.0f,
                        -1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

                        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
                        -1.0f, -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

                        // --- Left face
                        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, 0.5f, -1.0f, 0.0f,
                        0.0f, -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

                        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
                        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,

                        // --- Right face
                        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
                        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

                        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
                        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,

                        // --- Top face
                        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
                        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,

                        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
                        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,

                        // --- Bottom face
                        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.5f, -0.5f, 0.5f, 0.0f, -1.0f,
                        0.0f, -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,

                        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, -1.0f,
                        0.0f, 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f};

    mesh->create(vertices, 36, 6);
    return mesh;
}

dzemikk::Mesh* createQuadMesh() {
    dzemikk::Mesh* mesh = new dzemikk::Mesh();
    float vertices[] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
                        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    mesh->create(vertices, 6, 3);
    return mesh;
}

void createCubeBoard(std::shared_ptr<dzemikk::Engine> engine, dzemikk::Scene& scene,
                     dzemikk::Mesh* cubeMesh, dzemikk::Material* materialA,
                     dzemikk::Material* materialB, int rows, int cols, float spacing) {
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            auto cubeGO = scene.createGameObject();

            float x = -col * spacing;
            float y = 0.0f;
            float z = -row * spacing;

            cubeGO->transform()->setPosition(glm::vec3(x, y, z));
            cubeGO->transform()->setScale(glm::vec3(1.0f));

            auto cubeRenderer = cubeGO->addComponent<dzemikk::MeshRenderer>();
            cubeRenderer->setMesh(cubeMesh);
            cubeRenderer->setTransform(cubeGO->transform());
            cubeGO->transform()->setScale(glm::vec3(1.f));

            if ((row + col) % 2 == 0)
                cubeRenderer->setMaterial(materialA);
            else
                cubeRenderer->setMaterial(materialB);
        }
    }
}

dzemikk::Mesh* loadMeshFromFile(const std::string& path) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals |
                                                       aiProcess_JoinIdenticalVertices);

    if (!scene) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
    }

    if (!scene->HasMeshes()) {
        std::cerr << "NO MESHES IN FILE" << std::endl;
    }

    aiMesh* ai_mesh = scene->mMeshes[0];

    std::vector<float> vertices;

    for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++) {
        // pozycja
        vertices.push_back(ai_mesh->mVertices[i].x);
        vertices.push_back(ai_mesh->mVertices[i].y);
        vertices.push_back(ai_mesh->mVertices[i].z);

        // normal
        vertices.push_back(ai_mesh->mNormals[i].x);
        vertices.push_back(ai_mesh->mNormals[i].y);
        vertices.push_back(ai_mesh->mNormals[i].z);
    }

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < ai_mesh->mNumFaces; i++) {
        aiFace face = ai_mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    auto mesh = new dzemikk::Mesh();
    mesh->createIndexed(vertices.data(), ai_mesh->mNumVertices, indices.data(), indices.size(), 6);

    return mesh;
}

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

GLuint loadTextureFromFile(const std::string& path, bool flipVertical) {
    int width, height, channels;

    stbi_set_flip_vertically_on_load(flipVertical ? 1 : 0);

    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0; 
    }

    GLenum format = GL_RGB;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}