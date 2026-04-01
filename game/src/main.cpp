#include "core/engine.h"
#include "ecs/components/animator.h"
#include "ecs/components/camera.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"

#include <GLFW/glfw3.h>
#include <memory>

dzemikk::Mesh* createCubeMesh();
dzemikk::Mesh* createQuadMesh();
void createCubeBoard(std::shared_ptr<dzemikk::Engine> engine, dzemikk::Scene& scene,
                     dzemikk::Mesh* cubeMesh, dzemikk::Material* materialA,
                     dzemikk::Material* materialB, int rows, int cols, float spacing);

int main() {
    auto engine = std::make_shared<dzemikk::Engine>();

    dzemikk::Scene mainScene;
    auto playerGO = mainScene.createGameObject();
    playerGO->transform()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    playerGO->transform()->setRotation(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)));
    playerGO->transform()->setScale(glm::vec3(1.0f));
    dzemikk::Animator* animator =  playerGO->addComponent<dzemikk::Animator>();




    // --- Scene Camera
    auto cameraGO = mainScene.createGameObject();

    cameraGO->transform()->setPosition(glm::vec3(1.5f, 1.5f, 3.0f));

    auto camera = cameraGO->addComponent<dzemikk::Camera>();
    camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    // Rejestracja kamery w rendererze
    engine->getRenderer()->registerCamera(camera);
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
    materialA->shader = shaderA;

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
    materialB->shader = shaderB;

    auto cubeMesh = createCubeMesh();
    createCubeBoard(engine, mainScene, cubeMesh, materialA, materialB, 250, 250, 1.2f);

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
    uniform mat4 model;
    uniform mat4 projection;

    void main() {
        gl_Position = projection * model * vec4(aPos, 1.0);
    }
    )";

    const char* fragmentSrcUI = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.2,0.8,0.3,1.0); 
    }
    )";

    auto quadShader = new dzemikk::Shader(vertexSrcUI, fragmentSrcUI);
    auto quadMaterial = new dzemikk::Material();
    quadMaterial->shader = quadShader;

    auto quadRenderer = quadGO->addComponent<dzemikk::SpriteRenderer>();
    quadRenderer->mesh = quadMesh;
    quadRenderer->material = quadMaterial;
    quadRenderer->transform = quadGO->transform();

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

    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    mesh->vertexCount = 36;
    return mesh;
}

dzemikk::Mesh* createQuadMesh() {
    dzemikk::Mesh* mesh = new dzemikk::Mesh();
    float vertices[] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
                        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    mesh->vertexCount = 6;
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
            cubeRenderer->mesh = cubeMesh;
            cubeRenderer->transform = cubeGO->transform();

            if ((row + col) % 2 == 0)
                cubeRenderer->material = materialA;
            else
                cubeRenderer->material = materialB;
        }
    }
}