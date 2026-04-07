#include "core/engine.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/transform.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"

#include "core/input.h"
#include "core/time.h"

#include <memory>

dzemikk::Mesh* createCubeMesh();
dzemikk::Mesh* createQuadMesh();

int main() {
    auto engine = std::make_shared<dzemikk::Engine>();

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

    auto cubeTransform = new dzemikk::Transform();

    auto cubeRenderer = new dzemikk::MeshRenderer();
    cubeRenderer->mesh = cubeMesh;
    cubeRenderer->material = cubeMaterial;
    cubeRenderer->transform = cubeTransform;

    engine->GetRenderer()->registerRenderer(cubeRenderer);

    auto quadMesh = createQuadMesh();

    const char* vertexSrcUI = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * view * model * vec4(aPos,1.0);
    }
    )";

    const char* fragmentSrcUI = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.2,0.8,0.3,1.0); 
    }
    )";

    auto uiShader = new dzemikk::Shader(vertexSrcUI, fragmentSrcUI);
    auto quadMaterial = new dzemikk::Material();
    quadMaterial->shader = uiShader;

    auto quadTransform = new dzemikk::Transform();
    quadTransform->setPosition(glm::vec3(100.0f, 300.0f, 0.0f)); 
    quadTransform->setScale(glm::vec3(100.0f, 100.0f, 1.0f));   

    auto quadRenderer = new dzemikk::SpriteRenderer();
    quadRenderer->mesh = quadMesh;
    quadRenderer->material = quadMaterial;
    quadRenderer->transform = quadTransform;

    engine->GetRenderer()->registerSpriteRenderer(quadRenderer);

    glm::mat4 view =
        glm::lookAt(glm::vec3(1.5f, 1.5f, 3.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    engine->GetRenderer()->setCamera(view, projection);

    glm::mat4 uiOrtho = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
    engine->GetRenderer()->setUIProjection(uiOrtho);

    glm::vec2 lastMousePos = dzemikk::Input::GetMousePosition();

    engine->SetUserUpdateCallback([&]() {
        glm::vec2 currentMousePos = dzemikk::Input::GetMousePosition();
        glm::vec2 delta = currentMousePos - lastMousePos;
        lastMousePos = currentMousePos;

        // Jeśli wciśnięty Lewy Przycisk Myszy, obracaj sześcian myszką
        if (dzemikk::Input::IsMouseButtonPressed(dzemikk::Mouse::ButtonLeft)) {
            glm::vec3 rot = cubeTransform->getEulerAngles();
            rot.y += delta.x * 0.5f;
            rot.x += delta.y * 0.5f;
            cubeTransform->setEulerAngles(rot);
        }

        // Poruszanie za pomocą WSAD
        glm::vec3 pos = cubeTransform->getPosition();
        float speed = 2.0f * dzemikk::Time::deltaTime;

        if (dzemikk::Input::IsKeyPressed(dzemikk::Key::W)) pos.z -= speed;
        if (dzemikk::Input::IsKeyPressed(dzemikk::Key::S)) pos.z += speed;
        if (dzemikk::Input::IsKeyPressed(dzemikk::Key::A)) pos.x -= speed;
        if (dzemikk::Input::IsKeyPressed(dzemikk::Key::D)) pos.x += speed;

        cubeTransform->setPosition(pos);
    });

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
    float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f,  -0.5f, 0.0f, 0.5f,  0.5f,  0.0f,
                        0.5f,  0.5f,  0.0f, -0.5f, 0.5f,  0.0f, -0.5f, -0.5f, 0.0f};
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