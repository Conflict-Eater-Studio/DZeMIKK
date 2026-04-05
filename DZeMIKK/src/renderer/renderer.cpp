#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "ecs/componentRegistry.h"

#include "ecs/components/meshRenderer.h"
#include "ecs/components/spriteRenderer.h"
#include "ecs/components/camera.h"
#include "ecs/components/transform.h"
#include <iostream>
#include <map>

#include <filesystem>
#include <GLFW/glfw3.h>

void dzemikk::Renderer::Initialize() {
    _view = glm::mat4(1.0f);
    _projection = glm::mat4(1.0f);
    _uiProjection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

    glGenBuffers(1, &_uboMatrices);

    glBindBuffer(GL_UNIFORM_BUFFER, _uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, _uboMatrices, 0, 2 * sizeof(glm::mat4));

    
    glEnable(GL_MULTISAMPLE);

    _skybox = std::make_unique<Skybox>();
    
}

void dzemikk::Renderer::UnInitialize() {
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (_skybox && _sceneCamera) {

        float time = glfwGetTime(); 

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), time * 0.1f, glm::vec3(0, 1, 0));
        glm::mat4 viewNoTrans = _sceneCamera->getView() * rotation;

        _skybox->render(viewNoTrans, _sceneCamera->getProjection());
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
        float radius = 1.0f;

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
    }

    if (_uiCamera)
        _uiProjection = _uiCamera->getProjection();


    dzemikk::ComponentRegistry::get().getComponents<SpriteRenderer>(_spriteRenderers);

    for (auto* r : _spriteRenderers) {
        if (!r->isValid())
            continue;

        Shader* shader = r->getMaterial()->getShader();
        shader->bind();

        shader->setMat4("model", r->getTransform()->getWorldMatrix());
        shader->setMat4("projection", _uiProjection);

        r->getMesh()->draw();
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

void dzemikk::Renderer::setSkybox(std::unique_ptr<Skybox> skybox) {
    if (!skybox) {
        _skybox.reset();
        return;
    }
    _skybox = std::move(skybox);
}

const dzemikk::Skybox* dzemikk::Renderer::getSkybox() const {
    return _skybox.get();
}