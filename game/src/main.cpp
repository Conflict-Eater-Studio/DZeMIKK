#include "core/engine.h"
#include "ecs/components/camera.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "ecs/scene.h"

#include <memory>
#include <GLFW/glfw3.h>

dzemikk::Mesh* createCubeMesh();
dzemikk::Mesh* createQuadMesh();

int main() {
    auto engine = std::make_shared<dzemikk::Engine>();

    dzemikk::Scene mainScene;

    // --- Scene Camera
    auto cameraGO = mainScene.createGameObject();

    cameraGO->transform()->setPosition(glm::vec3(1.5f, 1.5f, 3.0f));

    auto camera = cameraGO->addComponent<dzemikk::Camera>();
    camera->lookAt(glm::vec3(0, 0, 0));

    // Rejestracja kamery w rendererze
    engine->GetRenderer()->registerCamera(camera);
    engine->GetRenderer()->setActiveSceneCamera(camera);

    // --- Cube GameObject
    auto cubeGO = new dzemikk::GameObject();

    auto cubeMesh = createCubeMesh();
    const char* vertexSrc3D = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * view * model * vec4(aPos,1.0);
    }
    )";

    const char* fragmentSrc3D = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0,0.5,0.2,1.0);
    }
    )";

    auto cubeShader = new dzemikk::Shader(vertexSrc3D, fragmentSrc3D);
    auto cubeMaterial = new dzemikk::Material();
    cubeMaterial->shader = cubeShader;

    auto cubeRenderer = cubeGO->addComponent<dzemikk::MeshRenderer>();
    cubeRenderer->mesh = cubeMesh;
    cubeRenderer->material = cubeMaterial;
    cubeRenderer->transform = cubeGO->transform();

    engine->GetRenderer()->registerRenderer(cubeRenderer);

    // UI Camera
    auto cameraUIGO = mainScene.createGameObject();
    cameraUIGO->transform()->setPosition(glm::vec3(0.0f, 0.0f, 1.0f));

    auto cameraUI = cameraUIGO->addComponent<dzemikk::Camera>();

    cameraUI->setOrthographic(0.0f, 1920.0f, 
                            0.0f, 1080.0f, 
                            -1.0f, 1.0f  
    );
    engine->GetRenderer()->setActiveUICamera(cameraUI);

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

    engine->GetRenderer()->registerSpriteRenderer(quadRenderer);

    engine->update();

    return 0;
}

dzemikk::Mesh* createCubeMesh() {
    dzemikk::Mesh* mesh = new dzemikk::Mesh();
    float vertices[] = {// front
                        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
                        -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f,
                        // back
                        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
                        -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
                        // left
                        -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
                        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
                        // right
                        0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f,
                        0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
                        // top
                        -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
                        -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f,
                        // bottom
                        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
                        0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f};
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
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