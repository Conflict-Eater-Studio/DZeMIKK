#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/font.h"
#include "renderer/texture.h"
#include "ecs/componentRegistry.h"

#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/textRenderer.h"
#include "ecs/components/camera.h"
#include "ecs/components/transform.h"
#include "ecs/gameobject.h"
#include "core/profiler.h"
#include <iostream>
#include <map>

#include <filesystem>
#include <GLFW/glfw3.h>

void dzemikk::Renderer::initialize() {
    _view = glm::mat4(1.0f);
    _projection = glm::mat4(1.0f);
    _uiProjection = glm::ortho(0.0f, 1920.0f, 0.0f, 1080.0f);

    glGenBuffers(1, &_uboMatrices);

    glBindBuffer(GL_UNIFORM_BUFFER, _uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, _uboMatrices, 0, 2 * sizeof(glm::mat4));

    
    glEnable(GL_MULTISAMPLE);

    _skybox = new Skybox();

    const char* vertexSrc = R"(
    #version 330 core
    layout (location = 0) in vec4 vertex; // pos.xy, uv.xy

    out vec2 TexCoords;

    uniform mat4 projection;

    void main() {
        gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
        TexCoords = vertex.zw;
    }
    )";

    const char* fragmentSrc = R"(
    #version 330 core
    in vec2 TexCoords;
    out vec4 color;

    uniform sampler2D text;
    uniform vec3 textColor;

    void main() {
        float alpha = texture(text, TexCoords).r;
        color = vec4(textColor, alpha);
    }
    )";

    _textShader = new Shader(vertexSrc, fragmentSrc);

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
}

void dzemikk::Renderer::uninitialize() {
    for (auto& batch : _batches) {
        if (batch.instanceVBO != 0) {
            glDeleteBuffers(1, &batch.instanceVBO);
            batch.instanceVBO = 0;
        }
    }

    if (_uboMatrices != 0) {
        glDeleteBuffers(1, &_uboMatrices);
        _uboMatrices = 0;
    }

    _batches.clear();
}

void dzemikk::Renderer::render() {
    Profiler::resetFrame();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (_skybox && _sceneCamera) {

        float time = glfwGetTime(); 

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), time * 0.1f, glm::vec3(0, 1, 0));
        glm::mat4 viewNoTrans = _sceneCamera->getView() * rotation;

        _skybox->render(viewNoTrans, _sceneCamera->getProjection());

        Profiler::rendererStats.drawCalls++;
        Profiler::rendererStats.renderedObjects++;
        Profiler::rendererStats.vertexCount += 36;
        Profiler::rendererStats.triangleCount += 12;
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    dzemikk::ComponentRegistry::get().getComponents<Camera>(_cameras);

    if (_sceneCamera) {
        if (_sceneCamera->isDirty()) {
            _frustum.update(_sceneCamera->getViewProjection());
        }

        _view = _sceneCamera->getView();
        _projection = _sceneCamera->getProjection();

        glm::mat4 matrices[2] = {_projection, _view};

        glBindBuffer(GL_UNIFORM_BUFFER, _uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(matrices), matrices);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    for (auto& batch : _batches) {
        batch.models.clear();
    }

    dzemikk::ComponentRegistry::get().getComponents<MeshRenderer>(_meshRenderers);

    for (auto* r : _meshRenderers) {
        if (!r->isValid())
            continue;

        glm::vec3 pos = r->getTransform()->getPosition();
        float radius = r->getCullingRadius();

        if (!_frustum.isSphereVisible(pos, radius))
            continue;

        Batch* batch = nullptr;

        for (auto& b : _batches) {
            if (b.mesh == r->getMesh() && b.material == r->getMaterial()) {
                batch = &b;
                break;
            }
        }

        if (!batch) {
            _batches.push_back({});
            batch = &_batches.back();
            batch->mesh = r->getMesh();
            batch->material = r->getMaterial();

            glGenBuffers(1, &batch->instanceVBO);
        }

        batch->models.push_back(r->getTransform()->getWorldMatrix());

        Profiler::rendererStats.renderedObjects++;
        Profiler::rendererStats.vertexCount += batch->mesh->getVertexCount();
        Profiler::rendererStats.triangleCount += batch->mesh->getVertexCount() / 3;
    }

    for (auto& batch : _batches) {
        if (batch.models.empty())
            continue;

        Mesh* mesh = batch.mesh;
        Material* material = batch.material;
        Shader* shader = material->getShader();

        shader->bind();

        shader->setVec3("lightDir", glm::vec3(1.0f, -1.0f, 1.0f));
        shader->setVec3("lightColor", glm::vec3(1.0f));
        shader->setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.2f));

        mesh->drawInstanced(batch.models, batch.instanceVBO);
        Profiler::rendererStats.drawCalls++;
    }

    if (_uiCamera)
        _uiProjection = _uiCamera->getProjection();


    dzemikk::ComponentRegistry::get().getComponents<SpriteRenderer>(_spriteRenderers);
    glDisable(GL_DEPTH_TEST);

for (auto* r : _spriteRenderers) {
        if (!r->isValid())
            continue;

        Shader* shader = r->getMaterial()->getShader();
        shader->bind();

        shader->setMat4("model", r->getTransform()->getWorldMatrix());
        shader->setMat4("projection", _uiProjection);

        if (r->hasTexture()) {
            r->getTexture()->bind(0);
            shader->setInt("spriteTexture", 0);
            shader->setBool("useTexture", true);
        } else {
            shader->setBool("useTexture", false);
        }

        shader->setVec4("spriteColor", r->getColor());

        r->getMesh()->draw();

        Profiler::rendererStats.drawCalls++;
        Profiler::rendererStats.renderedObjects++;
        Profiler::rendererStats.vertexCount += r->getMesh()->getVertexCount();
        Profiler::rendererStats.triangleCount += r->getMesh()->getVertexCount() / 3;
    }

    std::vector<TextRenderer*> texts;
    ComponentRegistry::get().getComponents<TextRenderer>(texts);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto* t : texts) {
        if (!t->isValid())
            continue;

        Shader* shader = _textShader;
        shader->bind();

        shader->setMat4("projection", _uiProjection);
        shader->setVec3("textColor", t->color);

        float x = t->getOwner()->transform()->getPosition().x;
        float y = t->getOwner()->transform()->getPosition().y;

        glBindVertexArray(textVAO);
        for (char c : t->text) {
            Character ch = t->font->characters[c];

            float xpos = x + ch.bearing.x * t->scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * t->scale;

            float w = ch.size.x * t->scale;
            float h = ch.size.y * t->scale;

            float vertices[6][4] = {{xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
                                    {xpos + w, ypos, 1.0f, 1.0f},

                                    {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
                                    {xpos + w, ypos + h, 1.0f, 0.0f}};

            glBindTexture(GL_TEXTURE_2D, ch.textureID);

            glBindBuffer(GL_ARRAY_BUFFER, textVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            Profiler::rendererStats.drawCalls++;

            Profiler::rendererStats.renderedObjects++;
            Profiler::rendererStats.vertexCount += 6;
            Profiler::rendererStats.triangleCount += 2;

            x += (ch.advance >> 6) * t->scale;
        }
        glBindVertexArray(0);
    }
}

const dzemikk::Camera* dzemikk::Renderer::getActiveSceneCamera() const {
    return _sceneCamera;
}
const dzemikk::Camera* dzemikk::Renderer::getActiveUICamera() const {
    return _uiCamera;
}

void dzemikk::Renderer::setActiveSceneCamera(dzemikk::Camera* camera) {
    if (!camera)
        return;

    _sceneCamera = camera;
}

void dzemikk::Renderer::setActiveUICamera(dzemikk::Camera* camera) {
    if (!camera)
        return;

    _uiCamera = camera;
}

void dzemikk::Renderer::setActiveSceneCameraById(int cameraId) {
    for (auto& cam : _cameras) {
        if (cam->getId() == cameraId) {
            _sceneCamera = cam;
            return;
        }
    }
    std::cerr << "[Renderer] Warning: Scene camera with ID " << cameraId << " not found.\n";
}

void dzemikk::Renderer::setActiveUICameraById(int cameraId) {
    for (auto& cam : _cameras) {
        if (cam->getId() == cameraId) {
            _uiCamera = cam;
            return;
        }
    }
    std::cerr << "[Renderer] Warning: UI camera with ID " << cameraId << " not found.\n";
}

void dzemikk::Renderer::setSkybox(Skybox* skybox) {
    if (!skybox) {
        _skybox = nullptr;
        return;
    }
    _skybox = skybox;
}

const dzemikk::Skybox* dzemikk::Renderer::getSkybox() const {
    return _skybox;
}